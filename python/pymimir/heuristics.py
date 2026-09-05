"""Public heuristic API."""

from __future__ import annotations

from abc import ABC, abstractmethod
from collections.abc import Sequence
import ctypes
import math
from numbers import Real
import threading
from typing import ClassVar, TypeAlias
import weakref

from .advanced import free_handle, lib
from .advanced._native import _Finalizer, _NativeOwner, _create_finalizer, raise_last_error
from .model import (
    GroundAction,
    GroundConjunctiveCondition,
    Problem,
    State,
    _handle_array,
)

Successors: TypeAlias = Sequence[tuple[GroundAction, State]]
Expansions: TypeAlias = Sequence[tuple[State, Successors]]


def _validate_value(value: object) -> float:
    if isinstance(value, bool) or not isinstance(value, Real):
        raise TypeError("heuristic values must be real numbers")
    result = float(value)
    if math.isnan(result) or result < 0.0 or result == -math.inf:
        raise ValueError("heuristic values must be nonnegative or positive infinity")
    return result


class Heuristic(ABC):
    """Base class for native and Python-defined heuristics."""

    _problem: Problem

    def __init__(self, problem: Problem) -> None:
        if not isinstance(problem, Problem):
            raise TypeError("problem must be a Problem")
        self._problem = problem

    @property
    def problem(self) -> Problem:
        return self._problem

    @abstractmethod
    def evaluate(
        self,
        state: State,
        goal: GroundConjunctiveCondition | None = None,
    ) -> float:
        ...

    def is_preferred(
        self,
        state: State,
        action: GroundAction,
        goal: GroundConjunctiveCondition | None = None,
    ) -> bool:
        return False

    def evaluate_batch(
        self,
        states: Sequence[State],
        goal: GroundConjunctiveCondition | None = None,
    ) -> Sequence[float]:
        return [self.evaluate(state, goal) for state in states]

    def preferred_actions(
        self,
        state: State,
        goal: GroundConjunctiveCondition | None = None,
    ) -> tuple[GroundAction, ...]:
        return tuple(
            action
            for action in state.applicable_actions()
            if self.is_preferred(state, action, goal)
        )


class _NativeHeuristic(_NativeOwner, Heuristic):
    _factory_name: ClassVar[str]
    _handle: int
    _finalizer: _Finalizer

    def __init__(self, problem: Problem) -> None:
        super().__init__(problem)
        factory = getattr(lib, self._factory_name)
        handle = int(factory(problem._handle))
        if handle == 0:
            raise_last_error(f"could not create {type(self).__name__}")
        finalizer = _create_finalizer(self, free_handle, handle)
        try:
            self._handle = handle
            self._finalizer = finalizer
        except BaseException:
            finalizer()
            raise

    def _effective_goal(
        self,
        goal: GroundConjunctiveCondition | None,
    ) -> GroundConjunctiveCondition:
        result = self.problem.goal if goal is None else goal
        if not isinstance(result, GroundConjunctiveCondition):
            raise TypeError("goal must be a GroundConjunctiveCondition")
        if result.problem != self.problem:
            raise ValueError("goal and heuristic belong to different problems")
        return result

    def _handle_for(self, _goal: GroundConjunctiveCondition) -> int:
        return self._handle

    def evaluate(
        self,
        state: State,
        goal: GroundConjunctiveCondition | None = None,
    ) -> float:
        if not isinstance(state, State):
            raise TypeError("state must be a State")
        if state.problem != self.problem:
            raise ValueError("state and heuristic belong to different problems")
        effective_goal = self._effective_goal(goal)
        heuristic_handle = self._handle_for(effective_goal)
        return _validate_value(lib.mimir_heuristic_evaluate(
            heuristic_handle,
            state._handle,
            effective_goal._handle,
        ))

    def preferred_actions(
        self,
        state: State,
        goal: GroundConjunctiveCondition | None = None,
    ) -> tuple[GroundAction, ...]:
        if not isinstance(state, State):
            raise TypeError("state must be a State")
        if state.problem != self.problem:
            raise ValueError("state and heuristic belong to different problems")
        effective_goal = self._effective_goal(goal)
        heuristic_handle = self._handle_for(effective_goal)
        list_handle = lib.mimir_heuristic_get_preferred_actions(
            heuristic_handle,
            state._handle,
            self.problem._handle,
            effective_goal._handle,
        )
        if list_handle == 0:
            raise_last_error("could not evaluate preferred actions")
        try:
            return tuple(
                GroundAction._from_handle(lib.mimir_action_list_get(list_handle, index), self.problem)
                for index in range(lib.mimir_action_list_count(list_handle))
            )
        finally:
            free_handle(list_handle)

    def evaluate_batch(
        self,
        states: Sequence[State],
        goal: GroundConjunctiveCondition | None = None,
    ) -> tuple[float, ...]:
        states = tuple(states)
        for state in states:
            if not isinstance(state, State):
                raise TypeError("states must contain only State values")
            if state.problem is not self.problem and state.problem != self.problem:
                raise ValueError("state and heuristic belong to different problems")
        effective_goal = self._effective_goal(goal)
        pointer, handles = _handle_array([state._handle for state in states])
        values = (ctypes.c_double * len(states))()
        if not lib.mimir_heuristic_evaluate_batch(
            self._handle_for(effective_goal), pointer, len(states), effective_goal._handle, values
        ):
            raise_last_error("could not evaluate heuristic batch")
        return tuple(_validate_value(value) for value in values)

    def is_preferred(
        self,
        state: State,
        action: GroundAction,
        goal: GroundConjunctiveCondition | None = None,
    ) -> bool:
        if not isinstance(action, GroundAction):
            raise TypeError("action must be a GroundAction")
        if action.problem != self.problem:
            raise ValueError("action and heuristic belong to different problems")
        return action in self.preferred_actions(state, goal)


class QHeuristic(ABC):
    """Score complete, ordered successor rows for a planning problem."""

    def __init__(self, problem: Problem) -> None:
        if not isinstance(problem, Problem):
            raise TypeError("problem must be a Problem")
        self._problem = problem

    @property
    def problem(self) -> Problem:
        return self._problem

    @abstractmethod
    def evaluate(
        self,
        state: State,
        successors: Successors,
        goal: GroundConjunctiveCondition | None = None,
    ) -> Sequence[float]:
        ...

    def evaluate_batch(
        self,
        expansions: Expansions,
        goal: GroundConjunctiveCondition | None = None,
    ) -> Sequence[Sequence[float]]:
        return [self.evaluate(state, successors, goal) for state, successors in expansions]


class BlindHeuristic(_NativeHeuristic):
    def __init__(self, problem: Problem) -> None:
        Heuristic.__init__(self, problem)
        handle = int(lib.mimir_heuristic_blind())
        if handle == 0:
            raise_last_error("could not create BlindHeuristic")
        finalizer = _create_finalizer(self, free_handle, handle)
        try:
            self._handle = handle
            self._finalizer = finalizer
        except BaseException:
            finalizer()
            raise


class GoalCountHeuristic(_NativeHeuristic):
    _factory_name = "mimir_heuristic_goal_count"


class FFHeuristic(_NativeHeuristic):
    _factory_name = "mimir_heuristic_ff_grounded"


class LiftedFFHeuristic(_NativeHeuristic):
    _factory_name = "mimir_heuristic_lifted_ff"


class AddHeuristic(_NativeHeuristic):
    _factory_name = "mimir_heuristic_add"


class MaxHeuristic(_NativeHeuristic):
    _factory_name = "mimir_heuristic_max"


class H2Heuristic(_NativeHeuristic):
    _factory_name = "mimir_heuristic_h2"


class SetAddHeuristic(_NativeHeuristic):
    _factory_name = "mimir_heuristic_set_add"


class _AlternatePerfectHandles(_NativeOwner):
    def __init__(self) -> None:
        self._lock = threading.RLock()
        # Goals compare structurally, but each native handle must follow the
        # lifetime of the specific Python wrapper that caused its creation.
        self._handles: dict[
            int,
            tuple[weakref.ReferenceType[GroundConjunctiveCondition], int],
        ] = {}
        self._closed = False

    def handle_for(self, problem: Problem, goal: GroundConjunctiveCondition) -> int:
        key = id(goal)
        cache_reference = weakref.ref(self)

        def release(
            reference: weakref.ReferenceType[GroundConjunctiveCondition],
        ) -> None:
            cache = cache_reference()
            if cache is not None:
                cache._release(key, reference)

        goal_reference = weakref.ref(goal, release)
        with self._lock:
            if self._closed:
                raise RuntimeError("PerfectHeuristic handle cache is closed")

            entry = self._handles.get(key)
            if entry is not None:
                cached_goal = entry[0]()
                if cached_goal is goal:
                    return entry[1]

                self._handles.pop(key)
                free_handle(entry[1])

            handle = int(lib.mimir_heuristic_perfect(problem._handle, goal._handle))
            if handle == 0:
                raise_last_error("could not create PerfectHeuristic")

            try:
                self._handles[key] = (goal_reference, handle)
            except BaseException:
                self._handles.pop(key, None)
                free_handle(handle)
                raise
            return handle

    def _release(
        self,
        key: int,
        reference: weakref.ReferenceType[GroundConjunctiveCondition],
    ) -> None:
        with self._lock:
            entry = self._handles.get(key)
            if entry is None or entry[0] is not reference:
                return
            self._handles.pop(key)
            handle = entry[1]
        free_handle(handle)

    def close(self) -> None:
        with self._lock:
            if self._closed:
                return
            self._closed = True
            handles = [entry[1] for entry in self._handles.values()]
            self._handles.clear()
        for handle in handles:
            free_handle(handle)


class PerfectHeuristic(_NativeHeuristic):
    def __init__(self, problem: Problem) -> None:
        Heuristic.__init__(self, problem)
        default_goal = problem.goal
        alternate_handles = _AlternatePerfectHandles()
        self._default_goal = default_goal
        self._alternate_handles = alternate_handles
        handle = int(lib.mimir_heuristic_perfect(
            problem._handle,
            default_goal._handle,
        ))
        if handle == 0:
            raise_last_error("could not create PerfectHeuristic")
        finalizer = _create_finalizer(
            self,
            _free_perfect_handles,
            handle,
            alternate_handles,
        )
        try:
            self._handle = handle
            self._finalizer = finalizer
        except BaseException:
            finalizer()
            raise

    def _effective_goal(
        self,
        goal: GroundConjunctiveCondition | None,
    ) -> GroundConjunctiveCondition:
        if goal is None:
            return self._default_goal
        return super()._effective_goal(goal)

    def _handle_for(self, goal: GroundConjunctiveCondition) -> int:
        if goal is self._default_goal or goal == self._default_goal:
            return self._handle
        return self._alternate_handles.handle_for(self.problem, goal)


def _free_perfect_handles(
    default_handle: int,
    alternate_handles: _AlternatePerfectHandles,
) -> None:
    alternate_handles.close()
    free_handle(default_handle)


__all__ = [
    "QHeuristic",
    "Successors",
    "Expansions",
    "Heuristic",
    "BlindHeuristic",
    "GoalCountHeuristic",
    "FFHeuristic",
    "LiftedFFHeuristic",
    "AddHeuristic",
    "MaxHeuristic",
    "H2Heuristic",
    "SetAddHeuristic",
    "PerfectHeuristic",
]
