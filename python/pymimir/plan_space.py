"""Plan-space planning: serial plans, their errors and the edits that change them."""

from __future__ import annotations

from collections.abc import Callable, Sequence
from dataclasses import dataclass
from enum import Enum

from .advanced import free_handle, lib, relationship_value
from .advanced._native import _Finalizer, _NativeOwner, _create_finalizer, raise_last_error
from .learning import _copy_ints, _int_pointer
from .model import Action, GroundAction, GroundAtom, GroundConjunctiveCondition, GroundLiteral, Object, Problem


@dataclass(frozen=True, slots=True)
class GhostStep:
    """An achiever of a literal: bound where the literal determines its arguments and ``None`` where they are free."""

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


def _full_binding(problem: Problem, indices: tuple[int, ...]) -> tuple[Object, ...]:
    objects = problem.all_objects
    return tuple(objects[index] for index in indices)


def _goal_handles(problem: Problem, goal: GroundConjunctiveCondition) -> list[int]:
    if not isinstance(goal, GroundConjunctiveCondition):
        raise TypeError("goal must be a GroundConjunctiveCondition")
    if goal.problem is not problem:
        raise ValueError("goal belongs to a different Problem than the plan")
    return [literal._handle for literal in goal.literals]


class SerialErrorKind(Enum):
    UNSATISFIED_PRECONDITION = 0
    UNMET_GOAL = 1


@dataclass(frozen=True, slots=True)
class SerialStep:
    id: int
    schema: Action
    binding: tuple[Object, ...]


@dataclass(frozen=True, slots=True)
class SerialError:
    """A false precondition of a step, or an unmet goal literal (``step_id`` -1), with the candidates that make it true.

    The candidates are indices into :attr:`SerialDiagnosis.candidates`.
    """

    kind: SerialErrorKind
    step_id: int
    literal: GroundLiteral
    candidate_indices: tuple[int, ...]
    # The step that last gave the atom the value the literal rejects, -1 when it has that value from the start.
    culprit_step_id: int


@dataclass(frozen=True, slots=True)
class SerialSupport:
    producer_step_id: int
    consumer_step_id: int
    literal: GroundLiteral


@dataclass(frozen=True, slots=True)
class SerialInsert:
    point: int
    schema: Action
    binding: tuple[Object, ...]


@dataclass(frozen=True, slots=True)
class SerialMove:
    step_id: int
    point: int


@dataclass(frozen=True, slots=True)
class SerialRebind:
    step_id: int
    binding: tuple[Object, ...]


@dataclass(frozen=True, slots=True)
class SerialEdits:
    """The edits of one round.

    All of them refer to the plan as it is before the round: a point is the
    insertion point before the step at that index (so a plan of ``n`` steps has
    ``n + 1`` points), and a step is named by its id. The new plan is read off
    point by point in the old order: the steps that enter at the point, inserted
    ones in the order given and then moved ones in their old order, followed by
    the old step after the point unless it was deleted or moved away. A rebound
    step keeps its id and gets its new binding wherever it ends up.
    """

    inserts: tuple[SerialInsert, ...] = ()
    deleted_step_ids: tuple[int, ...] = ()
    moves: tuple[SerialMove, ...] = ()
    rebinds: tuple[SerialRebind, ...] = ()


def _literal(handle: int, problem: Problem, what: str) -> GroundLiteral:
    if handle == 0:
        raise_last_error(f"could not read {what}")
    return GroundLiteral._from_handle(handle, problem)


class SerialPlan(_NativeOwner):
    """Native owner of one sequence of ground actions that need not be executable."""

    __slots__ = ("_handle", "_finalizer", "_problem", "__weakref__")

    _handle: int
    _finalizer: _Finalizer
    _problem: Problem

    def __init__(self) -> None:
        raise TypeError("use SerialPlan.empty()")

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
            steps.append(SerialStep(step_id, schema, _full_binding(self._problem, reader.take_counted())))
        reader.finish()
        return tuple(steps)

    def parameter_options(self, schema: Action, parameter_index: int) -> tuple[Object, ...]:
        """The objects a parameter of ``schema`` can be bound to, in the order of the problem's objects."""

        schema_index = self._problem.domain.actions.index(schema)
        count = int(lib.mimir_serial_plan_get_parameter_option_count(self._handle, schema_index, parameter_index))
        indices = _copy_ints(
            lambda handle, destination, capacity: lib.mimir_serial_plan_copy_parameter_options(
                handle, schema_index, parameter_index, destination, capacity
            ),
            self._handle,
            count,
            "parameter option",
        )
        return _full_binding(self._problem, tuple(indices))

    def diagnose(self) -> SerialDiagnosis:
        handle = int(lib.mimir_serial_plan_diagnose(self._handle))
        return SerialDiagnosis._from_handle(handle, self)

    def apply(self, edits: SerialEdits) -> None:
        """Change the plan by ``edits``; a diagnosis computed before no longer describes it."""

        if not isinstance(edits, SerialEdits):
            raise TypeError("edits must be SerialEdits")
        values = _edit_values(self._problem, edits)
        pointer, _array = _int_pointer(values)
        if int(lib.mimir_serial_plan_apply(self._handle, pointer, len(values))) != 1:
            raise_last_error("could not apply the plan edits")

    def preconditions(self, step_index: int) -> tuple[GroundLiteral, ...]:
        """The literals the step at ``step_index`` needs: the fluent preconditions of its schema, then the static ones.

        Two steps of one schema list their preconditions in the same order whatever their bindings are.
        """

        return self._step_literals(
            step_index, lib.mimir_serial_plan_get_step_precondition_count, lib.mimir_serial_plan_get_step_precondition, "precondition"
        )

    def effects(self, step_index: int) -> tuple[GroundLiteral, ...]:
        """The fluent literals the step at ``step_index`` makes true."""

        return self._step_literals(step_index, lib.mimir_serial_plan_get_step_effect_count, lib.mimir_serial_plan_get_step_effect, "effect")

    def _step_literals(
        self,
        step_index: int,
        count_function: Callable[[int, int], int],
        literal_function: Callable[[int, int, int], int],
        what: str,
    ) -> tuple[GroundLiteral, ...]:
        count = int(count_function(self._handle, step_index))
        if count < 0:
            raise_last_error(f"could not read the {what}s of step {step_index}")
        return tuple(
            _literal(int(literal_function(self._handle, step_index, index)), self._problem, f"a step {what}") for index in range(count)
        )

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
        return f"SerialPlan(steps={len(self.steps)})"


class SerialDiagnosis(_NativeOwner):
    """Snapshot of one plan's errors, read once, except for the final atoms."""

    __slots__ = (
        "_handle",
        "_finalizer",
        "_plan",
        "_errors",
        "_candidates",
        "_supports",
        "_unused_step_ids",
        "_final_atoms",
        "__weakref__",
    )

    _handle: int
    _finalizer: _Finalizer
    _plan: SerialPlan
    _errors: tuple[SerialError, ...]
    _candidates: tuple[GhostStep, ...]
    _supports: tuple[SerialSupport, ...]
    _unused_step_ids: tuple[int, ...]
    _final_atoms: tuple[GroundAtom, ...] | None

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
            diagnosis._final_atoms = None
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
    def final_atoms(self) -> tuple[GroundAtom, ...]:
        """The fluent atoms that are true after the last step, read on first use."""

        if self._final_atoms is None:
            count = int(lib.mimir_serial_diagnosis_get_final_fact_count(self._handle))
            if count < 0:
                raise_last_error("could not read the final atoms")
            self._final_atoms = tuple(
                GroundAtom._from_handle(lib.mimir_serial_diagnosis_get_final_fact(self._handle, index), self._plan.problem)
                for index in range(count)
            )
        return self._final_atoms

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
        reader = _read_values(
            lib.mimir_serial_diagnosis_get_error_value_count,
            lib.mimir_serial_diagnosis_copy_errors,
            self._handle,
            "serial error",
        )
        errors: list[SerialError] = []
        while reader.has_more():
            kind = SerialErrorKind(reader.next())
            step_id, culprit_step_id = reader.take(2)
            candidate_indices = reader.take_counted()
            literal_handle = int(lib.mimir_serial_diagnosis_get_error_literal(self._handle, len(errors)))
            literal = _literal(literal_handle, problem, "an error literal")
            errors.append(SerialError(kind, step_id, literal, candidate_indices, culprit_step_id))
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


def _edit_values(problem: Problem, edits: SerialEdits) -> list[int]:
    """The flat record the native side reads edits from."""

    object_indices = {value: index for index, value in enumerate(problem.all_objects)}
    schema_indices = {schema: index for index, schema in enumerate(problem.domain.actions)}

    def binding(values: Sequence[Object]) -> list[int]:
        return [len(values), *(object_indices[value] for value in values)]

    values = [len(edits.inserts)]
    for insert in edits.inserts:
        values += [insert.point, schema_indices[insert.schema], *binding(insert.binding)]
    values += [len(edits.deleted_step_ids), *edits.deleted_step_ids]
    values.append(len(edits.moves))
    for move in edits.moves:
        values += [move.step_id, move.point]
    values.append(len(edits.rebinds))
    for rebind in edits.rebinds:
        values += [rebind.step_id, *binding(rebind.binding)]
    return values


__all__ = [
    "GhostStep",
    "SerialErrorKind",
    "SerialStep",
    "SerialError",
    "SerialSupport",
    "SerialDiagnosis",
    "SerialInsert",
    "SerialMove",
    "SerialRebind",
    "SerialEdits",
    "SerialPlan",
]
