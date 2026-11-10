"""Plan-space planning: partial-order causal-link plans, their flaws and resolvers, and a graph encoding of them."""

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
from .model import Action, Domain, GroundAction, GroundAtom, GroundConjunctiveCondition, Object, Problem


class StepKind(Enum):
    INITIAL = 0
    GOAL = 1
    STEP = 2


class FlawKind(Enum):
    OPEN_CONDITION = 0
    THREAT = 1
    UNBOUND_PARAMETER = 2


class ResolverKind(Enum):
    NOOP = 0
    LINK_EXISTING = 1
    ADOPT_GHOST = 2
    PROMOTE = 3
    DEMOTE = 4
    DELETE_STEP = 5
    BIND = 6


@dataclass(frozen=True, slots=True)
class PlanStep:
    id: int
    kind: StepKind
    schema: Action | None
    binding: tuple[Object | None, ...]
    is_fully_bound: bool


@dataclass(frozen=True, slots=True)
class CausalLink:
    id: int
    producer_id: int
    consumer_id: int
    atom: GroundAtom


@dataclass(frozen=True, slots=True)
class Tombstone:
    schema: Action
    binding: tuple[Object, ...]
    age: int


@dataclass(frozen=True, slots=True)
class GhostStep:
    schema: Action
    binding: tuple[Object | None, ...]


@dataclass(frozen=True, slots=True)
class Resolver:
    kind: ResolverKind
    step_id: int
    ghost_index: int
    object: Object | None


@dataclass(frozen=True, slots=True)
class Flaw:
    kind: FlawKind
    step_id: int
    atom: GroundAtom | None
    link_id: int
    parameter_index: int
    resolvers: tuple[Resolver, ...]


@dataclass(frozen=True, slots=True)
class EditResult:
    applied_count: int
    dropped_flaw_indices: tuple[int, ...]
    created_step_ids: tuple[int, ...]
    deleted_step_ids: tuple[int, ...]


@dataclass(frozen=True, slots=True)
class OracleTargets:
    resolver_index_per_flaw: tuple[int, ...]
    step_ids_to_delete: tuple[int, ...]
    matched_step_count: int


@dataclass(frozen=True, slots=True)
class PlanEncodingLayout:
    step_node_ids: tuple[int, ...]
    parameter_node_ids: tuple[tuple[int, ...], ...]
    link_node_ids: tuple[int, ...]
    ghost_node_ids: tuple[int, ...]
    ghost_parameter_node_ids: tuple[tuple[int, ...], ...]
    flaw_node_ids: tuple[int, ...]
    resolver_node_ids: tuple[tuple[int, ...], ...]
    tombstone_node_ids: tuple[int, ...]


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


def _bound_binding(problem: Problem, indices: tuple[int, ...]) -> tuple[Object, ...]:
    objects = problem.all_objects
    for index in indices:
        if index < 0:
            raise RuntimeError("native returned an unbound argument where a bound one is required")
    return tuple(objects[index] for index in indices)


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


def _tombstone_options(tombstone_capacity: int, tombstone_lifespan: int) -> tuple[int, int]:
    if type(tombstone_capacity) is not int or type(tombstone_lifespan) is not int:
        raise TypeError("tombstone_capacity and tombstone_lifespan must be int")
    if tombstone_capacity < 0 or tombstone_lifespan < 0:
        raise ValueError("tombstone_capacity and tombstone_lifespan must be nonnegative")
    return tombstone_capacity, tombstone_lifespan


class PartialOrderPlan(_NativeOwner):
    """Native owner of one partial-order causal-link plan."""

    __slots__ = ("_handle", "_finalizer", "_problem", "__weakref__")

    _handle: int
    _finalizer: _Finalizer
    _problem: Problem

    def __init__(self) -> None:
        raise TypeError("use PartialOrderPlan.empty() or PartialOrderPlan.from_sequential_plan()")

    @classmethod
    def _from_handle(cls, handle: int, problem: Problem) -> PartialOrderPlan:
        if handle == 0:
            raise_last_error("could not create a partial-order plan")
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
    def empty(
        problem: Problem,
        tombstone_capacity: int = 0,
        tombstone_lifespan: int = 0,
        goal: GroundConjunctiveCondition | None = None,
    ) -> PartialOrderPlan:
        """The plan with only the initial and goal steps; ``goal`` defaults to the problem's goal."""

        if not isinstance(problem, Problem):
            raise TypeError("problem must be a Problem")
        capacity, lifespan = _tombstone_options(tombstone_capacity, tombstone_lifespan)
        if goal is None:
            handle = int(lib.mimir_plan_space_plan_empty(problem._handle, capacity, lifespan))
            return PartialOrderPlan._from_handle(handle, problem)
        goal_handles = _goal_handles(problem, goal)
        goal_pointer, _goal_array = _int_pointer(goal_handles)
        handle = int(
            lib.mimir_plan_space_plan_empty_with_goal(
                problem._handle,
                goal_pointer,
                len(goal_handles),
                capacity,
                lifespan,
            )
        )
        return PartialOrderPlan._from_handle(handle, problem)

    @staticmethod
    def from_sequential_plan(
        problem: Problem,
        actions: Sequence[GroundAction],
        tombstone_capacity: int = 0,
        tombstone_lifespan: int = 0,
        goal: GroundConjunctiveCondition | None = None,
    ) -> PartialOrderPlan:
        """The deordered plan of ``actions``; ``goal`` defaults to the problem's goal."""

        if not isinstance(problem, Problem):
            raise TypeError("problem must be a Problem")
        capacity, lifespan = _tombstone_options(tombstone_capacity, tombstone_lifespan)
        action_values = _sequence(actions, "actions")
        for action in action_values:
            if not isinstance(action, GroundAction):
                raise TypeError("actions must contain only GroundAction values")
            if action.problem is not problem:
                raise ValueError("actions contain a value from a different Problem")
        pointer, _array = _int_pointer([action._handle for action in action_values])
        if goal is None:
            handle = int(
                lib.mimir_plan_space_plan_from_sequential_plan(
                    problem._handle,
                    pointer,
                    len(action_values),
                    capacity,
                    lifespan,
                )
            )
            return PartialOrderPlan._from_handle(handle, problem)
        goal_handles = _goal_handles(problem, goal)
        goal_pointer, _goal_array = _int_pointer(goal_handles)
        handle = int(
            lib.mimir_plan_space_plan_from_sequential_plan_with_goal(
                problem._handle,
                pointer,
                len(action_values),
                goal_pointer,
                len(goal_handles),
                capacity,
                lifespan,
            )
        )
        return PartialOrderPlan._from_handle(handle, problem)

    @property
    def problem(self) -> Problem:
        return self._problem

    @property
    def goal(self) -> GroundConjunctiveCondition:
        """The goal this plan was built for, which may differ from the problem's goal."""

        count = int(lib.mimir_plan_space_plan_get_goal_count(self._handle))
        if count < 0:
            raise_last_error("native returned an invalid goal count")
        atoms: list[GroundAtom] = []
        for index in range(count):
            atom_handle = int(lib.mimir_plan_space_plan_get_goal_atom(self._handle, index))
            if atom_handle == 0:
                raise_last_error("could not read a goal atom")
            atoms.append(GroundAtom._from_handle(atom_handle, self._problem))
        return self._problem.ground_condition(*atoms)

    def with_goal(self, goal: GroundConjunctiveCondition) -> PartialOrderPlan:
        """A copy of this plan built for ``goal``: links for dropped atoms go, new atoms become open conditions."""

        goal_handles = _goal_handles(self._problem, goal)
        goal_pointer, _goal_array = _int_pointer(goal_handles)
        handle = int(lib.mimir_plan_space_plan_with_goal(self._handle, goal_pointer, len(goal_handles)))
        return PartialOrderPlan._from_handle(handle, self._problem)

    def prune_to_achieved_goal(self) -> PartialOrderPlan:
        """The largest flaw-free sub-plan of this plan, with its goal reduced to the atoms that sub-plan achieves."""

        handle = int(lib.mimir_plan_space_plan_prune_to_achieved_goal(self._handle))
        return PartialOrderPlan._from_handle(handle, self._problem)

    @property
    def round(self) -> int:
        value = int(lib.mimir_plan_space_plan_get_round(self._handle))
        if value < 0:
            raise_last_error("native returned an invalid plan round")
        return value

    @property
    def steps(self) -> tuple[PlanStep, ...]:
        reader = _read_values(
            lib.mimir_plan_space_plan_get_step_value_count,
            lib.mimir_plan_space_plan_copy_steps,
            self._handle,
            "plan step",
        )
        steps: list[PlanStep] = []
        while reader.has_more():
            step_id = reader.next()
            kind = StepKind(reader.next())
            schema_index = reader.next()
            is_fully_bound = reader.next() == 1
            binding = _binding(self._problem, reader.take_counted())
            schema = None if schema_index < 0 else _schema(self._problem, schema_index)
            steps.append(PlanStep(step_id, kind, schema, binding, is_fully_bound))
        reader.finish()
        return tuple(steps)

    @property
    def links(self) -> tuple[CausalLink, ...]:
        reader = _read_values(
            lib.mimir_plan_space_plan_get_link_value_count,
            lib.mimir_plan_space_plan_copy_links,
            self._handle,
            "plan link",
        )
        links: list[CausalLink] = []
        while reader.has_more():
            link_id, producer_id, consumer_id = reader.take(3)
            atom_handle = int(lib.mimir_plan_space_plan_get_link_atom(self._handle, len(links)))
            if atom_handle == 0:
                raise_last_error("could not read a causal-link atom")
            atom = GroundAtom._from_handle(atom_handle, self._problem)
            links.append(CausalLink(link_id, producer_id, consumer_id, atom))
        reader.finish()
        return tuple(links)

    @property
    def tombstones(self) -> tuple[Tombstone, ...]:
        reader = _read_values(
            lib.mimir_plan_space_plan_get_tombstone_value_count,
            lib.mimir_plan_space_plan_copy_tombstones,
            self._handle,
            "plan tombstone",
        )
        tombstones: list[Tombstone] = []
        while reader.has_more():
            schema = _schema(self._problem, reader.next())
            age = reader.next()
            binding = _bound_binding(self._problem, reader.take_counted())
            tombstones.append(Tombstone(schema, binding, age))
        reader.finish()
        return tuple(tombstones)

    def is_before(self, a: int, b: int) -> bool:
        if type(a) is not int or type(b) is not int:
            raise TypeError("step ids must be int")
        return relationship_value(
            lib.mimir_plan_space_plan_is_before(self._handle, a, b),
            "step ids are not part of the plan",
        )

    def ordering_closure(self) -> tuple[tuple[int, int], ...]:
        reader = _read_values(
            lib.mimir_plan_space_plan_get_ordering_value_count,
            lib.mimir_plan_space_plan_copy_ordering,
            self._handle,
            "plan ordering",
        )
        pairs: list[tuple[int, int]] = []
        while reader.has_more():
            before, after = reader.take(2)
            pairs.append((before, after))
        reader.finish()
        return tuple(pairs)

    def compute_flaws(self) -> FlawSet:
        handle = int(lib.mimir_plan_space_plan_compute_flaws(self._handle))
        return FlawSet._from_handle(handle, self)

    def apply(
        self,
        flaws: FlawSet,
        chosen_resolver_index: Sequence[int],
        step_ids_to_delete: Sequence[int],
    ) -> EditResult:
        if not isinstance(flaws, FlawSet):
            raise TypeError("flaws must be a FlawSet")
        if flaws.plan is not self:
            raise ValueError("flaws were computed from a different plan")
        resolver_indices = _int_sequence(chosen_resolver_index, "chosen_resolver_index")
        step_ids = _int_sequence(step_ids_to_delete, "step_ids_to_delete")
        resolver_pointer, _resolver_array = _int_pointer(resolver_indices)
        step_pointer, _step_array = _int_pointer(step_ids)
        handle = int(
            lib.mimir_plan_space_plan_apply(
                self._handle,
                flaws._handle,
                resolver_pointer,
                len(resolver_indices),
                step_pointer,
                len(step_ids),
            )
        )
        if handle == 0:
            raise_last_error("could not apply the plan edits")
        try:
            reader = _read_values(
                lib.mimir_plan_space_edit_result_get_value_count,
                lib.mimir_plan_space_edit_result_copy_values,
                handle,
                "edit result",
            )
        finally:
            free_handle(handle)
        applied_count = reader.next()
        dropped_count = reader.next()
        created_count = reader.next()
        deleted_count = reader.next()
        result = EditResult(
            applied_count,
            reader.take(dropped_count),
            reader.take(created_count),
            reader.take(deleted_count),
        )
        reader.finish()
        return result

    def is_solution(self) -> bool:
        return relationship_value(
            lib.mimir_plan_space_plan_is_solution(self._handle),
            "could not evaluate the plan",
        )

    def linearize(self) -> list[GroundAction]:
        handle = int(lib.mimir_plan_space_plan_linearize(self._handle))
        if handle == 0:
            raise_last_error("the plan cannot be linearized")
        try:
            count = int(lib.mimir_plan_space_action_list_get_count(handle))
            if count < 0:
                raise_last_error("native returned an invalid action count")
            actions: list[GroundAction] = []
            for index in range(count):
                action_handle = int(lib.mimir_plan_space_action_list_get_action(handle, index))
                if action_handle == 0:
                    raise_last_error("could not read a linearized action")
                actions.append(GroundAction._from_handle(action_handle, self._problem))
            return actions
        finally:
            free_handle(handle)

    def clone(self) -> PartialOrderPlan:
        handle = int(lib.mimir_plan_space_plan_clone(self._handle))
        return PartialOrderPlan._from_handle(handle, self._problem)

    def __repr__(self) -> str:
        return f"PartialOrderPlan(steps={len(self.steps)}, links={len(self.links)}, round={self.round})"


class FlawSet(_NativeOwner):
    """Snapshot of one plan's flaws; the native set stays alive because :meth:`PartialOrderPlan.apply` needs it."""

    __slots__ = ("_handle", "_finalizer", "_plan", "_flaws", "_ghosts", "__weakref__")

    _handle: int
    _finalizer: _Finalizer
    _plan: PartialOrderPlan
    _flaws: tuple[Flaw, ...]
    _ghosts: tuple[GhostStep, ...]

    def __init__(self) -> None:
        raise TypeError("use PartialOrderPlan.compute_flaws()")

    @classmethod
    def _from_handle(cls, handle: int, plan: PartialOrderPlan) -> FlawSet:
        if handle == 0:
            raise_last_error("could not compute the plan's flaws")
        try:
            flaws = object.__new__(cls)
            flaws._handle = handle
            flaws._plan = plan
        except BaseException:
            free_handle(handle)
            raise
        flaws._finalizer = _create_finalizer(flaws, free_handle, handle)
        flaws._ghosts = flaws._read_ghosts()
        flaws._flaws = flaws._read_flaws()
        return flaws

    @property
    def plan(self) -> PartialOrderPlan:
        return self._plan

    @property
    def flaws(self) -> tuple[Flaw, ...]:
        return self._flaws

    @property
    def ghosts(self) -> tuple[GhostStep, ...]:
        return self._ghosts

    def _read_ghosts(self) -> tuple[GhostStep, ...]:
        problem = self._plan.problem
        reader = _read_values(
            lib.mimir_plan_space_flaws_get_ghost_value_count,
            lib.mimir_plan_space_flaws_copy_ghosts,
            self._handle,
            "ghost step",
        )
        ghosts: list[GhostStep] = []
        while reader.has_more():
            schema = _schema(problem, reader.next())
            ghosts.append(GhostStep(schema, _binding(problem, reader.take_counted())))
        reader.finish()
        return tuple(ghosts)

    def _read_flaws(self) -> tuple[Flaw, ...]:
        problem = self._plan.problem
        objects = problem.all_objects
        reader = _read_values(
            lib.mimir_plan_space_flaws_get_value_count,
            lib.mimir_plan_space_flaws_copy_flaws,
            self._handle,
            "flaw",
        )
        flaws: list[Flaw] = []
        while reader.has_more():
            kind = FlawKind(reader.next())
            step_id = reader.next()
            link_id = reader.next()
            parameter_index = reader.next()
            resolver_count = reader.next()
            resolvers: list[Resolver] = []
            for _ in range(resolver_count):
                resolver_kind = ResolverKind(reader.next())
                resolver_step_id = reader.next()
                ghost_index = reader.next()
                object_index = reader.next()
                value = None if object_index < 0 else objects[object_index]
                resolvers.append(Resolver(resolver_kind, resolver_step_id, ghost_index, value))
            atom: GroundAtom | None = None
            if kind is not FlawKind.UNBOUND_PARAMETER:
                atom_handle = int(lib.mimir_plan_space_flaws_get_flaw_atom(self._handle, len(flaws)))
                if atom_handle == 0:
                    raise_last_error("could not read a flaw atom")
                atom = GroundAtom._from_handle(atom_handle, problem)
            flaws.append(Flaw(kind, step_id, atom, link_id, parameter_index, tuple(resolvers)))
        reader.finish()
        return tuple(flaws)

    def __len__(self) -> int:
        return len(self._flaws)

    def __repr__(self) -> str:
        return f"FlawSet(flaws={len(self._flaws)}, ghosts={len(self._ghosts)})"


def oracle_targets(
    current: PartialOrderPlan,
    flaws: FlawSet,
    reference: PartialOrderPlan,
) -> OracleTargets:
    """Resolver choices and deletions that move ``current`` towards ``reference``."""

    if not isinstance(current, PartialOrderPlan) or not isinstance(reference, PartialOrderPlan):
        raise TypeError("current and reference must be PartialOrderPlan values")
    if not isinstance(flaws, FlawSet):
        raise TypeError("flaws must be a FlawSet")
    if flaws.plan is not current:
        raise ValueError("flaws were computed from a different plan")
    if reference.problem is not current.problem:
        raise ValueError("the reference plan belongs to a different Problem")
    handle = int(lib.mimir_plan_space_oracle_targets(current._handle, flaws._handle, reference._handle))
    if handle == 0:
        raise_last_error("could not compute oracle targets")
    try:
        reader = _read_values(
            lib.mimir_plan_space_oracle_targets_get_value_count,
            lib.mimir_plan_space_oracle_targets_copy_values,
            handle,
            "oracle targets",
        )
    finally:
        free_handle(handle)
    matched_step_count = reader.next()
    flaw_count = reader.next()
    delete_count = reader.next()
    targets = OracleTargets(reader.take(flaw_count), reader.take(delete_count), matched_step_count)
    reader.finish()
    return targets


def encode_partial_plan(
    context: EncodingContext,
    plan: PartialOrderPlan,
    flaws: FlawSet,
    suffix: str = "",
) -> PlanEncodingLayout:
    """Append one partial plan with its flaws and resolvers to ``context`` and return the allocated node ids."""

    context_value = _require_context(context)
    if not isinstance(plan, PartialOrderPlan):
        raise TypeError("plan must be a PartialOrderPlan")
    if plan.problem is not context_value.problem:
        raise ValueError("plan belongs to a different Problem than the context")
    if not isinstance(flaws, FlawSet):
        raise TypeError("flaws must be a FlawSet")
    if flaws.plan is not plan:
        raise ValueError("flaws were computed from a different plan")
    handle = int(
        lib.mimir_plan_space_encode_partial_plan(
            context_value._handle,
            plan._handle,
            flaws._handle,
            _validate_suffix(suffix),
        )
    )
    if handle == 0:
        raise_last_error("native partial-plan encoding failed")
    try:
        reader = _read_values(
            lib.mimir_plan_space_layout_get_value_count,
            lib.mimir_plan_space_layout_copy_values,
            handle,
            "plan layout",
        )
    finally:
        free_handle(handle)
    step_node_ids = reader.take_counted()
    parameter_node_ids = tuple(reader.take_counted() for _ in step_node_ids)
    link_node_ids = reader.take_counted()
    ghost_node_ids = reader.take_counted()
    ghost_parameter_node_ids = tuple(reader.take_counted() for _ in ghost_node_ids)
    flaw_node_ids = reader.take_counted()
    resolver_node_ids = tuple(reader.take_counted() for _ in flaw_node_ids)
    tombstone_node_ids = reader.take_counted()
    reader.finish()
    return PlanEncodingLayout(
        step_node_ids,
        parameter_node_ids,
        link_node_ids,
        ghost_node_ids,
        ghost_parameter_node_ids,
        flaw_node_ids,
        resolver_node_ids,
        tombstone_node_ids,
    )


def partial_plan_relations(domain: Domain, suffix: str = "") -> list[tuple[str, int]]:
    """Every ``(name, arity)`` that :func:`encode_partial_plan` may emit for ``domain``."""

    if not isinstance(domain, Domain):
        raise TypeError("domain must be a Domain")
    text = take_string(lib.mimir_plan_space_relations(domain._handle, _validate_suffix(suffix)))
    if text is None:
        raise RuntimeError("native returned a null relation list")
    relations: list[tuple[str, int]] = []
    for line in text.split("\n"):
        name, _, arity = line.rpartition(" ")
        relations.append((name, int(arity)))
    return relations


__all__ = [
    "StepKind",
    "FlawKind",
    "ResolverKind",
    "PlanStep",
    "CausalLink",
    "Tombstone",
    "GhostStep",
    "Resolver",
    "Flaw",
    "FlawSet",
    "EditResult",
    "OracleTargets",
    "PartialOrderPlan",
    "oracle_targets",
    "PlanEncodingLayout",
    "encode_partial_plan",
    "partial_plan_relations",
]
