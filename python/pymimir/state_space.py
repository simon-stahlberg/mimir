"""Complete state-space inspection and sampling."""

from __future__ import annotations

from collections.abc import Iterator, Sequence
from dataclasses import dataclass
import random
from typing import overload
import weakref

from .advanced import free_handle, lib, relationship_value
from .advanced._native import _NativeOwner, _create_finalizer, raise_last_error
from .errors import MimirError
from .model import GroundAction, GroundConjunctiveCondition, Problem, State
from .search import Transition


@dataclass(frozen=True, slots=True)
class StateLabel:
    depth: int
    steps_to_goal: int | None
    cost_to_goal: float | None
    is_initial: bool
    is_goal: bool
    is_dead_end: bool


class StateSpace(_NativeOwner, Sequence[State]):
    def __init__(
        self,
        problem: Problem,
        *,
        goal: GroundConjunctiveCondition | None = None,
        max_states: int | None = None,
        seed: int | None = None,
    ) -> None:
        if not isinstance(problem, Problem):
            raise TypeError("problem must be a Problem")
        effective_goal = problem.goal if goal is None else goal
        if not isinstance(effective_goal, GroundConjunctiveCondition):
            raise TypeError("goal must be a GroundConjunctiveCondition")
        if effective_goal.problem != problem:
            raise ValueError("goal belongs to a different problem")
        if max_states is not None:
            if isinstance(max_states, bool) or not isinstance(max_states, int):
                raise TypeError("max_states must be int or None")
            if max_states <= 0:
                raise ValueError("max_states must be positive")
        if seed is not None and (isinstance(seed, bool) or not isinstance(seed, int)):
            raise TypeError("seed must be int or None")

        randomizer = random.Random(seed)
        handle = int(lib.mimir_state_space_create(
            problem._handle,
            effective_goal._handle,
            0 if max_states is None else max_states,
        ))
        if handle == 0:
            try:
                raise_last_error("could not construct a complete state space")
            except ValueError as error:
                raise MimirError(str(error)) from error
        finalizer = _create_finalizer(self, free_handle, handle)
        try:
            self._handle = handle
            self._problem = problem
            self._goal = effective_goal
            self._random = randomizer
            self._dead_end_indices: tuple[int, ...] | None = None
            self._distance_indices: dict[int, tuple[int, ...]] | None = None
            self._finalizer = finalizer
        except BaseException:
            finalizer()
            raise

    @property
    def problem(self) -> Problem:
        return self._problem

    @property
    def goal(self) -> GroundConjunctiveCondition:
        return self._goal

    @property
    def initial_state(self) -> State:
        handle = int(lib.mimir_state_space_get_initial_state(self._handle))
        if handle == 0:
            raise MimirError("state space has no initial state")
        return State._from_handle(handle, self._problem)

    @property
    def max_depth(self) -> int:
        return int(lib.mimir_state_space_max_depth(self._handle))

    @property
    def max_steps_to_goal(self) -> int | None:
        value = int(lib.mimir_state_space_max_distance_to_goal(self._handle))
        return None if value < 0 else value

    @property
    def transition_count(self) -> int:
        return int(lib.mimir_state_space_total_transitions(self._handle))

    @property
    def goal_state_count(self) -> int:
        return int(lib.mimir_state_space_goal_state_count(self._handle))

    @property
    def dead_end_state_count(self) -> int:
        return int(lib.mimir_state_space_dead_end_state_count(self._handle))

    @property
    def average_branching_factor(self) -> float:
        return float(lib.mimir_state_space_average_branching(self._handle))

    def __len__(self) -> int:
        return int(lib.mimir_state_space_total_states(self._handle))

    def __iter__(self) -> Iterator[State]:
        for index in range(len(self)):
            yield self[index]

    @overload
    def __getitem__(self, index: int) -> State: ...

    @overload
    def __getitem__(self, index: slice) -> tuple[State, ...]: ...

    def __getitem__(self, index: int | slice) -> State | tuple[State, ...]:
        if isinstance(index, slice):
            return tuple(self[position] for position in range(*index.indices(len(self))))
        if not isinstance(index, int):
            raise TypeError("state-space indices must be integers or slices")
        position = index + len(self) if index < 0 else index
        if position < 0 or position >= len(self):
            raise IndexError(index)
        handle = int(lib.mimir_state_space_get_state(self._handle, position))
        if handle == 0:
            raise MimirError("native state-space index returned no state")
        return State._from_handle(handle, self._problem)

    def _validate_state(self, state: State) -> int:
        if not isinstance(state, State):
            raise TypeError("state must be a State")
        if state.problem != self._problem:
            raise ValueError("state belongs to a different problem")
        depth = int(lib.mimir_state_space_get_depth(self._handle, state._handle))
        if depth < 0:
            raise ValueError("state is not present in this state space")
        return depth

    def label(self, state: State) -> StateLabel:
        depth = self._validate_state(state)
        is_goal = relationship_value(
            lib.mimir_state_space_is_goal(self._handle, state._handle),
            "could not query goal label",
        )
        is_dead_end = relationship_value(
            lib.mimir_state_space_is_dead_end(self._handle, state._handle),
            "could not query dead-end label",
        )
        steps = None if is_dead_end else int(
            lib.mimir_state_space_distance_to_goal(self._handle, state._handle))
        cost = None if is_dead_end else float(
            lib.mimir_state_space_cost_to_goal(self._handle, state._handle))
        return StateLabel(
            depth,
            steps,
            cost,
            depth == 0,
            is_goal,
            is_dead_end,
        )

    def _transitions(self, state: State, *, successors: bool) -> tuple[Transition, ...]:
        self._validate_state(state)
        function = (
            lib.mimir_state_space_get_successors
            if successors
            else lib.mimir_state_space_get_predecessors
        )
        list_handle = int(function(self._handle, state._handle))
        if list_handle == 0:
            raise MimirError("could not query state-space transitions")
        try:
            result = []
            for index in range(int(lib.mimir_transition_list_count(list_handle))):
                other = State._from_handle(
                    int(lib.mimir_transition_list_get_state(list_handle, index)),
                    self._problem,
                )
                action = GroundAction._from_handle(
                    int(lib.mimir_transition_list_get_action(list_handle, index)),
                    self._problem,
                )
                result.append(
                    Transition(state, action, other)
                    if successors
                    else Transition(other, action, state)
                )
            return tuple(result)
        finally:
            free_handle(list_handle)

    def successors(self, state: State) -> tuple[Transition, ...]:
        return self._transitions(state, successors=True)

    def predecessors(self, state: State) -> tuple[Transition, ...]:
        return self._transitions(state, successors=False)

    def reseed(self, seed: int | None) -> None:
        if seed is not None and (isinstance(seed, bool) or not isinstance(seed, int)):
            raise TypeError("seed must be int or None")
        self._random.seed(seed)

    def sample_state(self) -> State:
        if len(self) == 0:
            raise LookupError("state space is empty")
        return self[self._random.randrange(len(self))]

    def sample_states(self, count: int) -> tuple[State, ...]:
        self._validate_count(count)
        return tuple(self.sample_state() for _ in range(count))

    def sample_dead_end_state(self) -> State:
        dead_end_indices, _ = self._sampling_indices()
        if not dead_end_indices:
            raise LookupError("state space has no dead-end states")
        return self[self._random.choice(dead_end_indices)]

    def sample_dead_end_states(self, count: int) -> tuple[State, ...]:
        self._validate_count(count)
        return tuple(self.sample_dead_end_state() for _ in range(count))

    def sample_state_at_distance(self, distance: int) -> State:
        if isinstance(distance, bool) or not isinstance(distance, int):
            raise TypeError("distance must be int")
        if distance < 0:
            raise ValueError("distance must be nonnegative")
        _, distance_indices = self._sampling_indices()
        candidates = distance_indices.get(distance, ())
        if not candidates:
            raise LookupError(f"state space has no states at distance {distance}")
        return self[self._random.choice(candidates)]

    def sample_states_at_distance(self, distance: int, count: int) -> tuple[State, ...]:
        self._validate_count(count)
        return tuple(self.sample_state_at_distance(distance) for _ in range(count))

    def _sampling_indices(
        self,
    ) -> tuple[tuple[int, ...], dict[int, tuple[int, ...]]]:
        if self._dead_end_indices is not None and self._distance_indices is not None:
            return self._dead_end_indices, self._distance_indices

        dead_end_indices: list[int] = []
        distance_indices: dict[int, list[int]] = {}
        for index in range(len(self)):
            label = self.label(self[index])
            if label.is_dead_end:
                dead_end_indices.append(index)
                continue
            if label.steps_to_goal is None:
                raise RuntimeError("non-dead-end state has no distance-to-goal label")
            distance_indices.setdefault(label.steps_to_goal, []).append(index)

        self._dead_end_indices = tuple(dead_end_indices)
        self._distance_indices = {
            distance: tuple(indices)
            for distance, indices in distance_indices.items()
        }
        return self._dead_end_indices, self._distance_indices

    @staticmethod
    def _validate_count(count: int) -> None:
        if isinstance(count, bool) or not isinstance(count, int):
            raise TypeError("count must be int")
        if count < 0:
            raise ValueError("count must be nonnegative")


__all__ = ["StateLabel", "StateSpace"]
