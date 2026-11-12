"""Plan-space planning: serial plans with their errors and repairs, and their graph encoding."""

from __future__ import annotations

from collections.abc import Callable, Sequence
from dataclasses import dataclass
from enum import Enum

from .advanced import free_handle, lib, relationship_value, take_string
from .advanced._native import _Finalizer, _NativeOwner, _create_finalizer, raise_last_error
from .learning import (
    EncodingContext,
    _copy_ints,
    _int_pointer,
    _require_context,
    _sequence,
    _validate_suffix,
)
from .model import Action, Domain, GroundAction, GroundConjunctiveCondition, GroundLiteral, Object, Problem


@dataclass(frozen=True, slots=True)
class GhostStep:
    schema: Action
    binding: tuple[Object | None, ...]


class _Reader:
    """Sequential cursor over one flat int32 record array copied from native."""

    __slots__ = ("_values", "_position")

    def __init__(self, values: list[int]) -> None:
        self._values = values
        self._position = 0

    def next(self) -> int:
        if self._position >= len(self._values):
            raise RuntimeError("native record array ended prematurely")
        value = self._values[self._position]
        self._position += 1
        return value

    def take(self, count: int) -> tuple[int, ...]:
        if count < 0 or self._position + count > len(self._values):
            raise RuntimeError("native record array ended prematurely")
        values = tuple(self._values[self._position:self._position + count])
        self._position += count
        return values

    def take_counted(self) -> tuple[int, ...]:
        return self.take(self.next())

    def has_more(self) -> bool:
        return self._position < len(self._values)

    def finish(self) -> None:
        if self._position != len(self._values):
            raise RuntimeError("native record array has trailing values")


def _read_values(
    count_function: Callable[[int], int],
    copy_function: Callable[[int, object, int], int],
    handle: int,
    what: str,
) -> _Reader:
    count = int(count_function(handle))
    return _Reader(_copy_ints(copy_function, handle, count, what))


def _schema(problem: Problem, index: int) -> Action:
    return problem.domain.actions[index]


def _binding(problem: Problem, indices: tuple[int, ...]) -> tuple[Object | None, ...]:
    objects = problem.all_objects
    return tuple(None if index < 0 else objects[index] for index in indices)


def _int_sequence(values: Sequence[int], name: str) -> list[int]:
    result = list(_sequence(values, name))
    if any(type(value) is not int for value in result):
        raise TypeError(f"{name} must contain only integers")
    return result


def _goal_handles(problem: Problem, goal: GroundConjunctiveCondition) -> list[int]:
    if not isinstance(goal, GroundConjunctiveCondition):
        raise TypeError("goal must be a GroundConjunctiveCondition")
    if goal.problem is not problem:
        raise ValueError("goal belongs to a different Problem than the plan")
    return [literal._handle for literal in goal.literals]


class SerialErrorKind(Enum):
    UNSATISFIED_PRECONDITION = 0
    UNMET_GOAL = 1
    UNBOUND_PARAMETER = 2


class SerialRepairKind(Enum):
    NOOP = 0
    INSERT = 1
    BIND = 2
    REBIND = 3


@dataclass(frozen=True, slots=True)
class SerialStep:
    id: int
    schema: Action
    binding: tuple[Object | None, ...]
    is_fully_bound: bool
    purpose_step_id: int
    purpose: GroundLiteral


@dataclass(frozen=True, slots=True)
class SerialRepair:
    kind: SerialRepairKind
    candidate_index: int
    parameter_index: int
    object: Object | None


@dataclass(frozen=True, slots=True)
class SerialError:
    kind: SerialErrorKind
    step_id: int
    literal: GroundLiteral | None
    parameter_index: int
    repairs: tuple[SerialRepair, ...]


@dataclass(frozen=True, slots=True)
class SerialSupport:
    producer_step_id: int
    consumer_step_id: int
    literal: GroundLiteral


@dataclass(frozen=True, slots=True)
class SerialEditResult:
    applied_count: int
    dropped_error_indices: tuple[int, ...]
    created_step_ids: tuple[int, ...]
    deleted_step_ids: tuple[int, ...]


@dataclass(frozen=True, slots=True)
class SerialOracleTargets:
    repair_index_per_error: tuple[int, ...]
    step_ids_to_delete: tuple[int, ...]
    matched_step_count: int


@dataclass(frozen=True, slots=True)
class SerialEncodingLayout:
    initial_node_id: int
    goal_node_id: int
    step_node_ids: tuple[int, ...]
    parameter_node_ids: tuple[tuple[int, ...], ...]
    candidate_node_ids: tuple[int, ...]
    candidate_parameter_node_ids: tuple[tuple[int, ...], ...]
    error_node_ids: tuple[int, ...]
    repair_node_ids: tuple[tuple[int, ...], ...]


def _literal(handle: int, problem: Problem, what: str) -> GroundLiteral:
    if handle == 0:
        raise_last_error(f"could not read {what}")
    return GroundLiteral._from_handle(handle, problem)


class SerialPlan(_NativeOwner):
    """Native owner of one totally ordered plan whose steps may be partially bound and need not be executable."""

    __slots__ = ("_handle", "_finalizer", "_problem", "__weakref__")

    _handle: int
    _finalizer: _Finalizer
    _problem: Problem

    def __init__(self) -> None:
        raise TypeError("use SerialPlan.empty() or serial_search()")

    @classmethod
    def _from_handle(cls, handle: int, problem: Problem) -> SerialPlan:
        if handle == 0:
            raise_last_error("could not create a serial plan")
        try:
            plan = object.__new__(cls)
            plan._handle = handle
            plan._problem = problem
        except BaseException:
            free_handle(handle)
            raise
        plan._finalizer = _create_finalizer(plan, free_handle, handle)
        return plan

    @staticmethod
    def empty(problem: Problem, goal: GroundConjunctiveCondition | None = None) -> SerialPlan:
        """The plan without steps; ``goal`` defaults to the problem's goal."""

        if not isinstance(problem, Problem):
            raise TypeError("problem must be a Problem")
        if goal is None:
            return SerialPlan._from_handle(int(lib.mimir_serial_plan_empty(problem._handle)), problem)
        goal_handles = _goal_handles(problem, goal)
        goal_pointer, _goal_array = _int_pointer(goal_handles)
        handle = int(lib.mimir_serial_plan_empty_with_goal(problem._handle, goal_pointer, len(goal_handles)))
        return SerialPlan._from_handle(handle, problem)

    @property
    def problem(self) -> Problem:
        return self._problem

    @property
    def goal(self) -> GroundConjunctiveCondition:
        """The goal this plan is judged against, which may differ from the problem's goal."""

        count = int(lib.mimir_serial_plan_get_goal_count(self._handle))
        if count < 0:
            raise_last_error("native returned an invalid goal count")
        literals = [
            _literal(int(lib.mimir_serial_plan_get_goal_literal(self._handle, index)), self._problem, "a goal literal")
            for index in range(count)
        ]
        return self._problem.ground_condition(*literals)

    def with_goal(self, goal: GroundConjunctiveCondition) -> SerialPlan:
        """A copy of this plan with the same steps, judged against ``goal``."""

        goal_handles = _goal_handles(self._problem, goal)
        goal_pointer, _goal_array = _int_pointer(goal_handles)
        handle = int(lib.mimir_serial_plan_with_goal(self._handle, goal_pointer, len(goal_handles)))
        return SerialPlan._from_handle(handle, self._problem)

    def prune_to_achieved_goal(self) -> SerialPlan:
        """The executable sub-plan behind the goal literals this plan achieves, with its goal reduced to them."""

        handle = int(lib.mimir_serial_plan_prune_to_achieved_goal(self._handle))
        return SerialPlan._from_handle(handle, self._problem)

    @property
    def round(self) -> int:
        value = int(lib.mimir_serial_plan_get_round(self._handle))
        if value < 0:
            raise_last_error("native returned an invalid plan round")
        return value

    @property
    def steps(self) -> tuple[SerialStep, ...]:
        reader = _read_values(
            lib.mimir_serial_plan_get_step_value_count,
            lib.mimir_serial_plan_copy_steps,
            self._handle,
            "serial step",
        )
        steps: list[SerialStep] = []
        while reader.has_more():
            step_id = reader.next()
            schema = _schema(self._problem, reader.next())
            is_fully_bound = reader.next() == 1
            purpose_step_id = reader.next()
            binding = _binding(self._problem, reader.take_counted())
            purpose_handle = int(lib.mimir_serial_plan_get_step_purpose(self._handle, len(steps)))
            purpose = _literal(purpose_handle, self._problem, "a step purpose")
            steps.append(SerialStep(step_id, schema, binding, is_fully_bound, purpose_step_id, purpose))
        reader.finish()
        return tuple(steps)

    def diagnose(self) -> SerialDiagnosis:
        handle = int(lib.mimir_serial_plan_diagnose(self._handle))
        return SerialDiagnosis._from_handle(handle, self)

    def apply(
        self,
        diagnosis: SerialDiagnosis,
        chosen_repair_index: Sequence[int],
        step_ids_to_delete: Sequence[int],
    ) -> SerialEditResult:
        if not isinstance(diagnosis, SerialDiagnosis):
            raise TypeError("diagnosis must be a SerialDiagnosis")
        if diagnosis.plan is not self:
            raise ValueError("diagnosis was computed from a different plan")
        repair_indices = _int_sequence(chosen_repair_index, "chosen_repair_index")
        step_ids = _int_sequence(step_ids_to_delete, "step_ids_to_delete")
        repair_pointer, _repair_array = _int_pointer(repair_indices)
        step_pointer, _step_array = _int_pointer(step_ids)
        handle = int(
            lib.mimir_serial_plan_apply(
                self._handle,
                diagnosis._handle,
                repair_pointer,
                len(repair_indices),
                step_pointer,
                len(step_ids),
            )
        )
        if handle == 0:
            raise_last_error("could not apply the plan edits")
        try:
            reader = _read_values(
                lib.mimir_serial_edit_result_get_value_count,
                lib.mimir_serial_edit_result_copy_values,
                handle,
                "serial edit result",
            )
        finally:
            free_handle(handle)
        applied_count = reader.next()
        dropped_count = reader.next()
        created_count = reader.next()
        deleted_count = reader.next()
        result = SerialEditResult(
            applied_count,
            reader.take(dropped_count),
            reader.take(created_count),
            reader.take(deleted_count),
        )
        reader.finish()
        return result

    def is_solution(self) -> bool:
        return relationship_value(
            lib.mimir_serial_plan_is_solution(self._handle),
            "could not evaluate the plan",
        )

    def to_actions(self) -> list[GroundAction]:
        handle = int(lib.mimir_serial_plan_to_actions(self._handle))
        if handle == 0:
            raise_last_error("the plan still has errors")
        try:
            count = int(lib.mimir_serial_action_list_get_count(handle))
            if count < 0:
                raise_last_error("native returned an invalid action count")
            actions: list[GroundAction] = []
            for index in range(count):
                action_handle = int(lib.mimir_serial_action_list_get_action(handle, index))
                if action_handle == 0:
                    raise_last_error("could not read a plan action")
                actions.append(GroundAction._from_handle(action_handle, self._problem))
            return actions
        finally:
            free_handle(handle)

    def clone(self) -> SerialPlan:
        handle = int(lib.mimir_serial_plan_clone(self._handle))
        return SerialPlan._from_handle(handle, self._problem)

    def __repr__(self) -> str:
        return f"SerialPlan(steps={len(self.steps)}, round={self.round})"


class SerialDiagnosis(_NativeOwner):
    """Snapshot of one plan's errors, read once; the native diagnosis stays alive because
    :meth:`SerialPlan.apply`, :func:`serial_oracle_targets` and :func:`encode_serial_plan` need it."""

    __slots__ = (
        "_handle",
        "_finalizer",
        "_plan",
        "_errors",
        "_candidates",
        "_supports",
        "_unused_step_ids",
        "__weakref__",
    )

    _handle: int
    _finalizer: _Finalizer
    _plan: SerialPlan
    _errors: tuple[SerialError, ...]
    _candidates: tuple[GhostStep, ...]
    _supports: tuple[SerialSupport, ...]
    _unused_step_ids: tuple[int, ...]

    def __init__(self) -> None:
        raise TypeError("use SerialPlan.diagnose()")

    @classmethod
    def _from_handle(cls, handle: int, plan: SerialPlan) -> SerialDiagnosis:
        if handle == 0:
            raise_last_error("could not diagnose the plan")
        try:
            diagnosis = object.__new__(cls)
            diagnosis._handle = handle
            diagnosis._plan = plan
        except BaseException:
            free_handle(handle)
            raise
        diagnosis._finalizer = _create_finalizer(diagnosis, free_handle, handle)
        diagnosis._candidates = diagnosis._read_candidates()
        diagnosis._errors = diagnosis._read_errors()
        diagnosis._supports = diagnosis._read_supports()
        unused_count = int(lib.mimir_serial_diagnosis_get_unused_step_count(handle))
        diagnosis._unused_step_ids = tuple(
            _copy_ints(lib.mimir_serial_diagnosis_copy_unused_steps, handle, unused_count, "unused step")
        )
        return diagnosis

    @property
    def plan(self) -> SerialPlan:
        return self._plan

    @property
    def errors(self) -> tuple[SerialError, ...]:
        return self._errors

    @property
    def candidates(self) -> tuple[GhostStep, ...]:
        return self._candidates

    @property
    def supports(self) -> tuple[SerialSupport, ...]:
        return self._supports

    @property
    def unused_step_ids(self) -> tuple[int, ...]:
        return self._unused_step_ids

    @property
    def is_solution(self) -> bool:
        return not self._errors

    def _read_candidates(self) -> tuple[GhostStep, ...]:
        problem = self._plan.problem
        reader = _read_values(
            lib.mimir_serial_diagnosis_get_candidate_value_count,
            lib.mimir_serial_diagnosis_copy_candidates,
            self._handle,
            "candidate step",
        )
        candidates: list[GhostStep] = []
        while reader.has_more():
            schema = _schema(problem, reader.next())
            candidates.append(GhostStep(schema, _binding(problem, reader.take_counted())))
        reader.finish()
        return tuple(candidates)

    def _read_errors(self) -> tuple[SerialError, ...]:
        problem = self._plan.problem
        objects = problem.all_objects
        reader = _read_values(
            lib.mimir_serial_diagnosis_get_error_value_count,
            lib.mimir_serial_diagnosis_copy_errors,
            self._handle,
            "serial error",
        )
        errors: list[SerialError] = []
        while reader.has_more():
            kind = SerialErrorKind(reader.next())
            step_id = reader.next()
            parameter_index = reader.next()
            repair_count = reader.next()
            repairs: list[SerialRepair] = []
            for _ in range(repair_count):
                repair_kind = SerialRepairKind(reader.next())
                candidate_index = reader.next()
                repair_parameter_index = reader.next()
                object_index = reader.next()
                value = None if object_index < 0 else objects[object_index]
                repairs.append(SerialRepair(repair_kind, candidate_index, repair_parameter_index, value))
            literal: GroundLiteral | None = None
            if kind is not SerialErrorKind.UNBOUND_PARAMETER:
                literal_handle = int(lib.mimir_serial_diagnosis_get_error_literal(self._handle, len(errors)))
                literal = _literal(literal_handle, problem, "an error literal")
            errors.append(SerialError(kind, step_id, literal, parameter_index, tuple(repairs)))
        reader.finish()
        return tuple(errors)

    def _read_supports(self) -> tuple[SerialSupport, ...]:
        reader = _read_values(
            lib.mimir_serial_diagnosis_get_support_value_count,
            lib.mimir_serial_diagnosis_copy_supports,
            self._handle,
            "serial support",
        )
        supports: list[SerialSupport] = []
        while reader.has_more():
            producer_step_id, consumer_step_id = reader.take(2)
            literal_handle = int(lib.mimir_serial_diagnosis_get_support_literal(self._handle, len(supports)))
            literal = _literal(literal_handle, self._plan.problem, "a support literal")
            supports.append(SerialSupport(producer_step_id, consumer_step_id, literal))
        reader.finish()
        return tuple(supports)

    def __len__(self) -> int:
        return len(self._errors)

    def __repr__(self) -> str:
        return f"SerialDiagnosis(errors={len(self._errors)}, candidates={len(self._candidates)})"


def serial_search(
    problem: Problem,
    max_expansions: int = 100_000,
    timeout_seconds: float = 10.0,
) -> SerialPlan | None:
    """A solution found by searching over single repairs from the empty plan, or ``None`` when the budget runs out."""

    if not isinstance(problem, Problem):
        raise TypeError("problem must be a Problem")
    if type(max_expansions) is not int:
        raise TypeError("max_expansions must be int")
    if isinstance(timeout_seconds, bool) or not isinstance(timeout_seconds, (int, float)):
        raise TypeError("timeout_seconds must be a number")
    if max_expansions < 0 or timeout_seconds < 0:
        raise ValueError("max_expansions and timeout_seconds must be nonnegative")
    # Native errors are raised by the call itself, so a null handle means that no plan was found.
    handle = int(lib.mimir_serial_search(problem._handle, max_expansions, float(timeout_seconds)))
    if handle == 0:
        return None
    return SerialPlan._from_handle(handle, problem)


def serial_oracle_targets(
    current: SerialPlan,
    diagnosis: SerialDiagnosis,
    reference: SerialPlan,
) -> SerialOracleTargets:
    """Repair choices and deletions that move ``current`` towards ``reference``."""

    if not isinstance(current, SerialPlan) or not isinstance(reference, SerialPlan):
        raise TypeError("current and reference must be SerialPlan values")
    if not isinstance(diagnosis, SerialDiagnosis):
        raise TypeError("diagnosis must be a SerialDiagnosis")
    if diagnosis.plan is not current:
        raise ValueError("diagnosis was computed from a different plan")
    if reference.problem is not current.problem:
        raise ValueError("the reference plan belongs to a different Problem")
    handle = int(lib.mimir_serial_oracle_targets(current._handle, diagnosis._handle, reference._handle))
    if handle == 0:
        raise_last_error("could not compute oracle targets")
    try:
        reader = _read_values(
            lib.mimir_serial_oracle_targets_get_value_count,
            lib.mimir_serial_oracle_targets_copy_values,
            handle,
            "serial oracle targets",
        )
    finally:
        free_handle(handle)
    matched_step_count = reader.next()
    error_count = reader.next()
    delete_count = reader.next()
    targets = SerialOracleTargets(reader.take(error_count), reader.take(delete_count), matched_step_count)
    reader.finish()
    return targets


def encode_serial_plan(
    context: EncodingContext,
    plan: SerialPlan,
    diagnosis: SerialDiagnosis,
    suffix: str = "",
) -> SerialEncodingLayout:
    """Append one serial plan with its errors and repairs to ``context`` and return the allocated node ids."""

    context_value = _require_context(context)
    if not isinstance(plan, SerialPlan):
        raise TypeError("plan must be a SerialPlan")
    if plan.problem is not context_value.problem:
        raise ValueError("plan belongs to a different Problem than the context")
    if not isinstance(diagnosis, SerialDiagnosis):
        raise TypeError("diagnosis must be a SerialDiagnosis")
    if diagnosis.plan is not plan:
        raise ValueError("diagnosis was computed from a different plan")
    handle = int(
        lib.mimir_serial_encode_plan(
            context_value._handle,
            plan._handle,
            diagnosis._handle,
            _validate_suffix(suffix),
        )
    )
    if handle == 0:
        raise_last_error("native serial-plan encoding failed")
    try:
        reader = _read_values(
            lib.mimir_serial_layout_get_value_count,
            lib.mimir_serial_layout_copy_values,
            handle,
            "serial layout",
        )
    finally:
        free_handle(handle)
    initial_node_id = reader.next()
    goal_node_id = reader.next()
    step_node_ids = reader.take_counted()
    parameter_node_ids = tuple(reader.take_counted() for _ in step_node_ids)
    candidate_node_ids = reader.take_counted()
    candidate_parameter_node_ids = tuple(reader.take_counted() for _ in candidate_node_ids)
    error_node_ids = reader.take_counted()
    repair_node_ids = tuple(reader.take_counted() for _ in error_node_ids)
    reader.finish()
    return SerialEncodingLayout(
        initial_node_id,
        goal_node_id,
        step_node_ids,
        parameter_node_ids,
        candidate_node_ids,
        candidate_parameter_node_ids,
        error_node_ids,
        repair_node_ids,
    )


def serial_plan_relations(domain: Domain, suffix: str = "") -> list[tuple[str, int]]:
    """Every ``(name, arity)`` that :func:`encode_serial_plan` may emit for ``domain``."""

    if not isinstance(domain, Domain):
        raise TypeError("domain must be a Domain")
    text = take_string(lib.mimir_serial_relations(domain._handle, _validate_suffix(suffix)))
    if text is None:
        raise RuntimeError("native returned a null relation list")
    relations: list[tuple[str, int]] = []
    for line in text.split("\n"):
        name, _, arity = line.rpartition(" ")
        relations.append((name, int(arity)))
    return relations


__all__ = [
    "GhostStep",
    "SerialErrorKind",
    "SerialRepairKind",
    "SerialStep",
    "SerialRepair",
    "SerialError",
    "SerialSupport",
    "SerialDiagnosis",
    "SerialEditResult",
    "SerialOracleTargets",
    "SerialPlan",
    "serial_search",
    "serial_oracle_targets",
    "SerialEncodingLayout",
    "encode_serial_plan",
    "serial_plan_relations",
]
