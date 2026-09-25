"""Framework-independent native graph encodings for planning data."""

from __future__ import annotations

import ctypes
from collections.abc import Callable, Sequence
from dataclasses import dataclass
from types import TracebackType
from typing import TypeVar

from .advanced import free_handle, lib, take_string
from .advanced._native import (
    _Finalizer,
    _NativeOwner,
    _create_finalizer,
    raise_last_error,
)
from .model import (
    GroundAction,
    GroundConjunctiveCondition,
    GroundLiteral,
    Object,
    Problem,
    State,
)


_T = TypeVar("_T")
_INT32_MIN = -(2**31)
_INT32_MAX = 2**31 - 1
_INT32_SIZE = ctypes.sizeof(ctypes.c_int32)
if _INT32_SIZE != 4:
    raise RuntimeError("pymimir.learning requires a four-byte ctypes.c_int32")


def _sequence(values: Sequence[_T], name: str) -> tuple[_T, ...]:
    if isinstance(values, (str, bytes)) or not isinstance(values, Sequence):
        raise TypeError(f"{name} must be a sequence")
    return tuple(values)


def _int_pointer(values: Sequence[int]) -> tuple[ctypes.c_void_p, object | None]:
    if not values:
        return ctypes.c_void_p(), None
    for value in values:
        if type(value) is not int:
            raise TypeError("native int32 buffers require integer values")
        if value < _INT32_MIN or value > _INT32_MAX:
            raise OverflowError(f"integer value {value} does not fit in int32")
    array = (ctypes.c_int * len(values))(*values)
    return ctypes.cast(array, ctypes.c_void_p), array


def _validate_suffix(suffix: str) -> bytes:
    if not isinstance(suffix, str):
        raise TypeError("suffix must be str")
    if "\0" in suffix:
        raise ValueError("suffix cannot contain a null character")
    return suffix.encode("utf-8")


def _copy_ints(
    function: Callable[[int, ctypes.c_void_p, int], int],
    handle: int,
    count: int,
    what: str,
) -> list[int]:
    if count < 0:
        raise_last_error(f"native returned a negative {what} count")
    if count == 0:
        copied = int(function(handle, ctypes.c_void_p(), 0))
        if copied != 0:
            raise RuntimeError(f"native returned an invalid {what} copy count")
        return []

    values = (ctypes.c_int * count)()
    pointer = ctypes.cast(values, ctypes.c_void_p)
    copied = int(function(handle, pointer, count))
    if copied != count:
        raise_last_error(
            f"native copied {copied} {what} values, expected {count}"
        )
    return values[:]


def _require_success(value: int, operation: str) -> None:
    if int(value) != 1:
        raise_last_error(operation)


@dataclass(frozen=True, slots=True)
class RelationDescriptor:
    """Location of one flattened relation in a :class:`RelationBuffer`.

    ``offset`` and ``length`` are measured in int32 values, not bytes.
    """

    name: str
    offset: int
    length: int

    def __post_init__(self) -> None:
        if not isinstance(self.name, str):
            raise TypeError("relation descriptor name must be str")
        if type(self.offset) is not int or type(self.length) is not int:
            raise TypeError("relation descriptor offset and length must be int")
        if self.offset < 0 or self.length < 0:
            raise ValueError(
                "relation descriptor offset and length must be nonnegative"
            )


class RelationBuffer:
    """Python-owned packed int32 relation values and their immutable layout.

    The writable :attr:`values` memoryview cannot be resized, so descriptor
    offsets remain valid. Objects such as CPU tensors created from this buffer
    retain the memoryview, and therefore the underlying storage.
    """

    __slots__ = ("_storage", "_pin", "_descriptors")

    _storage: bytearray
    _pin: memoryview
    _descriptors: tuple[RelationDescriptor, ...]

    def __init__(
        self,
        values: bytearray,
        descriptors: Sequence[RelationDescriptor],
    ) -> None:
        if not isinstance(values, bytearray):
            raise TypeError("relation buffer values must be a bytearray")
        descriptor_values = _sequence(descriptors, "descriptors")
        if not all(
            isinstance(descriptor, RelationDescriptor)
            for descriptor in descriptor_values
        ):
            raise TypeError("descriptors must contain RelationDescriptor values")
        if len(values) % _INT32_SIZE != 0:
            raise ValueError("relation buffer byte length must be a multiple of four")

        expected_offset = 0
        names: set[str] = set()
        for descriptor in descriptor_values:
            if descriptor.name in names:
                raise ValueError(
                    f"relation buffer contains duplicate name {descriptor.name!r}"
                )
            if descriptor.offset != expected_offset:
                raise ValueError("relation descriptors must be contiguous and ordered")
            if descriptor.length > _INT32_MAX - expected_offset:
                raise OverflowError(
                    "a relation buffer cannot exceed Int32.MaxValue values"
                )
            names.add(descriptor.name)
            expected_offset += descriptor.length
        if expected_offset != len(values) // _INT32_SIZE:
            raise ValueError("relation descriptors do not cover the packed values")

        self._storage = values
        # Keep one private export alive to prevent resizing the bytearray. The
        # public property returns a child view that callers may safely release.
        self._pin = memoryview(values)
        self._descriptors = descriptor_values

    @property
    def values(self) -> memoryview:
        """Writable byte view accepted directly by ``torch.frombuffer``."""

        return memoryview(self._pin)

    @property
    def descriptors(self) -> tuple[RelationDescriptor, ...]:
        return self._descriptors

    def to_relations(self) -> dict[str, list[int]]:
        """Materialize the packed values as RGNN-compatible Python lists."""

        value_count = len(self._storage) // _INT32_SIZE
        if value_count == 0:
            return {descriptor.name: [] for descriptor in self._descriptors}
        values = (ctypes.c_int32 * value_count).from_buffer(self._storage)
        return {
            descriptor.name: values[
                descriptor.offset:descriptor.offset + descriptor.length
            ]
            for descriptor in self._descriptors
        }


class EncodingContext(_NativeOwner):
    """Native owner for one complete graph-encoding batch."""

    __slots__ = (
        "_handle",
        "_finalizer",
        "_current_problem",
        "_object_to_id",
        "_auxiliary_ids",
        "__weakref__",
    )

    _handle: int
    _finalizer: _Finalizer
    _current_problem: Problem | None
    _object_to_id: dict[Object, int] | None
    _auxiliary_ids: dict[object, int]

    def __init__(self) -> None:
        handle = int(lib.mimir_learning_encoding_context_create())
        if handle == 0:
            raise_last_error("could not create learning EncodingContext")

        self._handle = handle
        self._finalizer = _create_finalizer(self, free_handle, handle)
        self._current_problem = None
        self._object_to_id = None
        self._auxiliary_ids = {}

    def __enter__(self) -> EncodingContext:
        self._require_open()
        return self

    def __exit__(
        self,
        _exception_type: type[BaseException] | None,
        _exception: BaseException | None,
        _traceback: TracebackType | None,
    ) -> None:
        self.close()

    def close(self) -> None:
        """Release the native context; repeated calls are safe."""

        if self._handle == 0:
            return

        self._handle = 0
        self._current_problem = None
        if self._object_to_id is not None:
            self._object_to_id.clear()
        self._object_to_id = None
        self._auxiliary_ids.clear()
        self._finalizer()

    def begin_instance(self, problem: Problem) -> None:
        self._require_open()
        if not isinstance(problem, Problem):
            raise TypeError("problem must be a Problem")
        if self._current_problem is not None:
            raise RuntimeError("an encoding instance is already active")

        _require_success(
            lib.mimir_learning_encoding_context_begin_instance(
                self._handle,
                problem._handle,
            ),
            "could not begin an encoding instance",
        )
        self._current_problem = problem
        self._object_to_id = None
        self._auxiliary_ids = {}

    def end_instance(self) -> None:
        self._require_active()
        _require_success(
            lib.mimir_learning_encoding_context_end_instance(self._handle),
            "could not end the encoding instance",
        )
        self._current_problem = None
        self._object_to_id = None
        self._auxiliary_ids = {}

    @property
    def problem(self) -> Problem:
        self._require_active()
        assert self._current_problem is not None
        return self._current_problem

    @property
    def object_to_id(self) -> dict[Object, int]:
        self._require_active()
        return dict(self._ensure_object_map())

    @property
    def id_offset(self) -> int:
        return self._current_count(
            lib.mimir_learning_encoding_context_get_current_node_offset,
            "node offset",
        )

    @property
    def batch_count(self) -> int:
        return self._count(
            lib.mimir_learning_encoding_context_get_batch_count,
            "batch",
        )

    @property
    def node_count(self) -> int:
        return self._count(
            lib.mimir_learning_encoding_context_get_node_count,
            "node",
        )

    @property
    def node_sizes(self) -> list[int]:
        return self._sizes(
            lib.mimir_learning_encoding_context_copy_node_sizes,
            "node size",
        )

    @property
    def object_sizes(self) -> list[int]:
        return self._sizes(
            lib.mimir_learning_encoding_context_copy_object_sizes,
            "object size",
        )

    @property
    def object_indices(self) -> list[int]:
        return self._indices(
            self.object_sizes,
            lib.mimir_learning_encoding_context_copy_object_indices,
            "object index",
        )

    @property
    def action_sizes(self) -> list[int]:
        return self._sizes(
            lib.mimir_learning_encoding_context_copy_action_sizes,
            "action size",
        )

    @property
    def action_indices(self) -> list[int]:
        return self._indices(
            self.action_sizes,
            lib.mimir_learning_encoding_context_copy_action_indices,
            "action index",
        )

    @property
    def virtual_sizes(self) -> list[int]:
        return self._sizes(
            lib.mimir_learning_encoding_context_copy_virtual_sizes,
            "virtual size",
        )

    @property
    def virtual_indices(self) -> list[int]:
        return self._indices(
            self.virtual_sizes,
            lib.mimir_learning_encoding_context_copy_virtual_indices,
            "virtual index",
        )

    @property
    def auxiliary_sizes(self) -> list[int]:
        return self._sizes(
            lib.mimir_learning_encoding_context_copy_auxiliary_sizes,
            "auxiliary size",
        )

    @property
    def auxiliary_indices(self) -> list[int]:
        return self._indices(
            self.auxiliary_sizes,
            lib.mimir_learning_encoding_context_copy_auxiliary_indices,
            "auxiliary index",
        )

    def get_object_id(self, value: Object) -> int:
        self._require_active()
        if not isinstance(value, Object):
            raise TypeError("value must be an Object")
        result = int(
            lib.mimir_learning_encoding_context_get_object_id(
                self._handle,
                value._handle,
            )
        )
        if result < 0:
            raise_last_error("native returned an invalid object node ID")
        return result

    def new_action_id(self) -> int:
        return self._allocate(
            lib.mimir_learning_encoding_context_new_action_id,
            "action",
        )

    def new_virtual_id(self) -> int:
        return self._allocate(
            lib.mimir_learning_encoding_context_new_virtual_id,
            "virtual",
        )

    def new_or_existing_virtual_id(self) -> int:
        return self._allocate(
            lib.mimir_learning_encoding_context_new_or_existing_virtual_id,
            "virtual",
        )

    def new_or_existing_auxiliary_id(self, key: object) -> int:
        self._require_active()
        if key in self._auxiliary_ids:
            return self._auxiliary_ids[key]

        if (
            isinstance(key, tuple)
            and len(key) == 2
            and isinstance(key[0], Object)
            and isinstance(key[1], Object)
        ):
            result = int(
                lib.mimir_learning_encoding_context_new_or_existing_object_pair_id(
                    self._handle,
                    key[0]._handle,
                    key[1]._handle,
                )
            )
            if result < 0:
                raise_last_error("could not allocate an object-pair node ID")
        else:
            result = self._allocate(
                lib.mimir_learning_encoding_context_new_auxiliary_id,
                "auxiliary",
            )

        self._auxiliary_ids[key] = result
        return result

    def get_auxiliary_id(self, key: object) -> int:
        self._require_active()
        try:
            return self._auxiliary_ids[key]
        except KeyError:
            pass

        if (
            isinstance(key, tuple)
            and len(key) == 2
            and isinstance(key[0], Object)
            and isinstance(key[1], Object)
        ):
            result = int(
                lib.mimir_learning_encoding_context_try_get_object_pair_id(
                    self._handle,
                    key[0]._handle,
                    key[1]._handle,
                )
            )
            if result >= 0:
                self._auxiliary_ids[key] = result
                return result
            if result < -1:
                raise_last_error("could not look up an object-pair node ID")
        raise KeyError(key)

    def get_object_ids(self) -> list[int]:
        self._require_active()
        return list(self._ensure_object_map().values())

    def get_object_count(self) -> int:
        return self._current_count(
            lib.mimir_learning_encoding_context_get_current_object_count,
            "object",
        )

    def get_action_ids(self) -> list[int]:
        return self._current_ids(
            lib.mimir_learning_encoding_context_get_current_action_count,
            lib.mimir_learning_encoding_context_copy_current_action_ids,
            "action",
        )

    def get_action_count(self) -> int:
        return self._current_count(
            lib.mimir_learning_encoding_context_get_current_action_count,
            "action",
        )

    def get_virtual_ids(self) -> list[int]:
        return self._current_ids(
            lib.mimir_learning_encoding_context_get_current_virtual_count,
            lib.mimir_learning_encoding_context_copy_current_virtual_ids,
            "virtual",
        )

    def get_virtual_count(self) -> int:
        return self._current_count(
            lib.mimir_learning_encoding_context_get_current_virtual_count,
            "virtual",
        )

    def get_auxiliary_ids(self) -> list[int]:
        return self._current_ids(
            lib.mimir_learning_encoding_context_get_current_auxiliary_count,
            lib.mimir_learning_encoding_context_copy_current_auxiliary_ids,
            "auxiliary",
        )

    def get_auxiliary_count(self) -> int:
        return self._current_count(
            lib.mimir_learning_encoding_context_get_current_auxiliary_count,
            "auxiliary",
        )

    def get_node_count(self) -> int:
        return self._current_count(
            lib.mimir_learning_encoding_context_get_current_node_count,
            "node",
        )

    def to_relation_buffer(self) -> RelationBuffer:
        """Copy every relation into one Python-owned packed int32 buffer."""

        self._require_open()
        if self._current_problem is not None:
            raise RuntimeError("cannot materialize relations while an instance is active")

        relation_count = self._count(
            lib.mimir_learning_encoding_context_get_relation_count,
            "relation",
        )
        descriptors: list[RelationDescriptor] = []
        names: set[str] = set()
        value_offset = 0
        for relation_index in range(relation_count):
            name = take_string(
                lib.mimir_learning_encoding_context_get_relation_name(
                    self._handle,
                    relation_index,
                )
            )
            if name is None:
                raise RuntimeError("native returned a null relation name")
            value_count = int(
                lib.mimir_learning_encoding_context_get_relation_value_count(
                    self._handle,
                    relation_index,
                )
            )
            if value_count < 0:
                raise_last_error(
                    f"native returned an invalid value count for relation {name!r}"
                )
            if value_count > _INT32_MAX - value_offset:
                raise OverflowError(
                    "the packed relation buffer cannot exceed Int32.MaxValue values"
                )
            if name in names:
                raise RuntimeError(f"native returned duplicate relation name {name!r}")
            names.add(name)
            descriptors.append(RelationDescriptor(name, value_offset, value_count))
            value_offset += value_count

        storage = bytearray(value_offset * _INT32_SIZE)
        if value_offset == 0:
            destination = ctypes.c_void_p()
            destination_owner = None
        else:
            destination_owner = (ctypes.c_int32 * value_offset).from_buffer(storage)
            destination = ctypes.cast(destination_owner, ctypes.c_void_p)
        copied = int(
            lib.mimir_learning_encoding_context_copy_all_relation_values(
                self._handle,
                destination,
                value_offset,
            )
        )
        # The native call only borrows the destination synchronously. Keep its
        # ctypes exporter alive explicitly through the call.
        del destination_owner
        if copied != value_offset:
            raise_last_error(
                f"native copied {copied} relation values, expected {value_offset}"
            )
        return RelationBuffer(storage, descriptors)

    def to_relations(self) -> dict[str, list[int]]:
        """Materialize every relation as an independent Python integer list."""

        return self.to_relation_buffer().to_relations()

    def _require_active(self) -> None:
        self._require_open()
        if self._current_problem is None:
            raise RuntimeError("no encoding instance is active")

    def _require_open(self) -> None:
        if self._handle == 0:
            raise RuntimeError("EncodingContext is closed")

    def _ensure_object_map(self) -> dict[Object, int]:
        self._require_active()
        if self._object_to_id is not None:
            return self._object_to_id

        assert self._current_problem is not None
        objects = self._current_problem.all_objects
        object_count = self.get_object_count()
        if len(objects) != object_count:
            raise RuntimeError(
                f"native context has {object_count} objects, expected {len(objects)}"
            )
        object_ids = _copy_ints(
            lib.mimir_learning_encoding_context_copy_current_object_ids,
            self._handle,
            object_count,
            "current object ID",
        )
        self._object_to_id = dict(zip(objects, object_ids))
        return self._object_to_id

    def _count(self, function: Callable[[int], int], what: str) -> int:
        self._require_open()
        result = int(function(self._handle))
        if result < 0:
            raise_last_error(f"native returned an invalid {what} count")
        return result

    def _current_count(self, function: Callable[[int], int], what: str) -> int:
        self._require_active()
        return self._count(function, f"current {what}")

    def _sizes(
        self,
        function: Callable[[int, ctypes.c_void_p, int], int],
        what: str,
    ) -> list[int]:
        if self._current_problem is not None:
            raise RuntimeError("cannot read batch metadata while an instance is active")
        return _copy_ints(function, self._handle, self.batch_count, what)

    def _indices(
        self,
        sizes: Sequence[int],
        function: Callable[[int, ctypes.c_void_p, int], int],
        what: str,
    ) -> list[int]:
        return _copy_ints(function, self._handle, sum(sizes), what)

    def _current_ids(
        self,
        count_function: Callable[[int], int],
        copy_function: Callable[[int, ctypes.c_void_p, int], int],
        what: str,
    ) -> list[int]:
        count = self._current_count(count_function, what)
        return _copy_ints(
            copy_function,
            self._handle,
            count,
            f"current {what} ID",
        )

    def _allocate(self, function: Callable[[int], int], category: str) -> int:
        self._require_active()
        result = int(function(self._handle))
        if result < 0:
            raise_last_error(f"could not allocate a {category} node ID")
        return result


def _require_context(context: EncodingContext) -> EncodingContext:
    if not isinstance(context, EncodingContext):
        raise TypeError("context must be an EncodingContext")
    context._require_active()
    return context


def _state_for_context(context: EncodingContext, state: State) -> State:
    if not isinstance(state, State):
        raise TypeError("state must be a State")
    if state.problem is not context.problem:
        raise ValueError("state belongs to a different Problem than the context")
    return state


def _goal_literals(
    context: EncodingContext,
    goal: GroundConjunctiveCondition,
) -> tuple[GroundLiteral, ...]:
    if not isinstance(goal, GroundConjunctiveCondition):
        raise TypeError("goal must be a GroundConjunctiveCondition")
    if goal.problem is not context.problem:
        raise ValueError("goal belongs to a different Problem than the context")
    return goal.literals


def encode_state(
    context: EncodingContext,
    state: State,
    *,
    suffix: str = "",
) -> None:
    """Append one state's relations to ``context``."""

    context_value = _require_context(context)
    state_value = _state_for_context(context_value, state)
    _require_success(
        lib.mimir_learning_encode_state(
            context_value._handle,
            state_value._handle,
            _validate_suffix(suffix),
        ),
        "native state encoding failed",
    )


def encode_goal(
    context: EncodingContext,
    state: State,
    goal: GroundConjunctiveCondition,
    *,
    suffix: str = "",
) -> None:
    """Append one positive goal split by current truth to ``context``."""

    context_value = _require_context(context)
    state_value = _state_for_context(context_value, state)
    literals = _goal_literals(context_value, goal)
    literal_pointer, _literal_array = _int_pointer(
        [literal._handle for literal in literals]
    )
    _require_success(
        lib.mimir_learning_encode_goal(
            context_value._handle,
            state_value._handle,
            literal_pointer,
            len(literals),
            _validate_suffix(suffix),
        ),
        "native goal encoding failed",
    )


def encode_action_list(
    context: EncodingContext,
    state: State,
    actions: Sequence[GroundAction],
    *,
    suffix: str = "",
) -> None:
    """Allocate and append one state's ordered ground-action list."""

    context_value = _require_context(context)
    state_value = _state_for_context(context_value, state)
    action_values = _sequence(actions, "actions")
    for action in action_values:
        if not isinstance(action, GroundAction):
            raise TypeError("actions must contain only GroundAction values")
        if action.problem is not context_value.problem:
            raise ValueError("actions contain a value from a different Problem")
    action_pointer, _action_array = _int_pointer(
        [action._handle for action in action_values]
    )
    _require_success(
        lib.mimir_learning_encode_action_list(
            context_value._handle,
            state_value._handle,
            action_pointer,
            len(action_values),
            _validate_suffix(suffix),
        ),
        "native action-list encoding failed",
    )


def encode_transition_effects(
    context: EncodingContext,
    source: State,
    successors: Sequence[State],
    actions: Sequence[GroundAction],
    effect_relations: Sequence[tuple[int, int]],
    goal: GroundConjunctiveCondition,
    *,
    suffix: str = "",
) -> None:
    """Append source-to-successor net effects, applied action names, and ordered transition links.

    ``actions[i]`` is the action whose application produced ``successors[i]``.
    """

    context_value = _require_context(context)
    source_value = _state_for_context(context_value, source)
    successor_values = _sequence(successors, "successors")
    for successor in successor_values:
        if not isinstance(successor, State):
            raise TypeError("successors must contain only State values")
        if successor.problem is not context_value.problem:
            raise ValueError("successors contain a value from a different Problem")

    action_values = _sequence(actions, "actions")
    if len(action_values) != len(successor_values):
        raise ValueError("actions must contain one action per successor")
    for action in action_values:
        if not isinstance(action, GroundAction):
            raise TypeError("actions must contain only GroundAction values")
        if action.problem is not context_value.problem:
            raise ValueError("actions contain a value from a different Problem")

    relation_values = _sequence(effect_relations, "effect_relations")
    flat_relation_indices: list[int] = []
    for index, pair in enumerate(relation_values):
        values = _sequence(pair, f"effect_relations[{index}]")
        if len(values) != 2 or any(type(value) is not int for value in values):
            raise TypeError("effect relations must be pairs of integers")
        flat_relation_indices.extend(values)

    goal_literals = _goal_literals(context_value, goal)
    successor_pointer, _successor_array = _int_pointer(
        [successor._handle for successor in successor_values]
    )
    action_pointer, _action_array = _int_pointer(
        [action._handle for action in action_values]
    )
    relation_pointer, _relation_array = _int_pointer(flat_relation_indices)
    goal_pointer, _goal_array = _int_pointer(
        [literal._handle for literal in goal_literals]
    )
    _require_success(
        lib.mimir_learning_encode_transition_effects(
            context_value._handle,
            source_value._handle,
            successor_pointer,
            len(successor_values),
            action_pointer,
            relation_pointer,
            len(relation_values),
            goal_pointer,
            len(goal_literals),
            _validate_suffix(suffix),
        ),
        "native transition-effects encoding failed",
    )


def encode_virtual_node(context: EncodingContext) -> None:
    """Append one virtual node linked to the active problem's declared objects."""

    context_value = _require_context(context)
    _require_success(
        lib.mimir_learning_encode_virtual_node(context_value._handle),
        "native virtual-node encoding failed",
    )


def encode_expressive_state(
    context: EncodingContext,
    state: State,
    *,
    suffix: str = "",
) -> None:
    """Append one ordered-object-pair state encoding."""

    context_value = _require_context(context)
    state_value = _state_for_context(context_value, state)
    _require_success(
        lib.mimir_learning_encode_expressive_state(
            context_value._handle,
            state_value._handle,
            _validate_suffix(suffix),
        ),
        "native expressive-state encoding failed",
    )


def encode_expressive_goal(
    context: EncodingContext,
    state: State,
    goal: GroundConjunctiveCondition,
    *,
    suffix: str = "",
) -> None:
    """Append one ordered-object-pair goal encoding."""

    context_value = _require_context(context)
    state_value = _state_for_context(context_value, state)
    literals = _goal_literals(context_value, goal)
    literal_pointer, _literal_array = _int_pointer(
        [literal._handle for literal in literals]
    )
    _require_success(
        lib.mimir_learning_encode_expressive_goal(
            context_value._handle,
            state_value._handle,
            literal_pointer,
            len(literals),
            _validate_suffix(suffix),
        ),
        "native expressive-goal encoding failed",
    )


__all__ = [
    "EncodingContext",
    "RelationBuffer",
    "RelationDescriptor",
    "encode_state",
    "encode_goal",
    "encode_action_list",
    "encode_transition_effects",
    "encode_virtual_node",
    "encode_expressive_state",
    "encode_expressive_goal",
]
