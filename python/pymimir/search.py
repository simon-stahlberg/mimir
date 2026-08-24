"""Public search functions."""

from __future__ import annotations

import ctypes
from _ctypes import _Pointer
from dataclasses import dataclass
from enum import Enum
import math
from types import TracebackType
from typing import Callable, TypeAlias

from .advanced import free_handle, lib
from .advanced._native import raise_last_error
from .advanced.heuristics import HEURISTIC_CALLBACK, PREFERRED_CALLBACK
from .advanced.search import DOUBLE_CALLBACK, STATE_CALLBACK, TRANSITION_CALLBACK
from .errors import MimirError
from .heuristics import Heuristic, PerfectHeuristic, _NativeHeuristic, _validate_value
from .model import GroundAction, GroundConjunctiveCondition, Problem, State


class SearchStatus(str, Enum):
    SOLVED = "solved"
    EXHAUSTED = "exhausted"
    TIMED_OUT = "timed_out"
    EXPANSION_LIMIT_REACHED = "expansion_limit_reached"


@dataclass(frozen=True, slots=True)
class Transition:
    source: State
    action: GroundAction
    target: State


@dataclass(frozen=True, slots=True)
class SearchStatistics:
    expanded_states: int
    generated_states: int
    max_depth: int
    setup_seconds: float
    search_seconds: float

    @property
    def elapsed_seconds(self) -> float:
        return self.setup_seconds + self.search_seconds


@dataclass(frozen=True, slots=True)
class Solution:
    plan: tuple[GroundAction, ...]
    cost: float
    goal_state: State


@dataclass(frozen=True, slots=True)
class SearchResult:
    status: SearchStatus
    solution: Solution | None
    statistics: SearchStatistics

    def __post_init__(self) -> None:
        if self.status is SearchStatus.SOLVED and self.solution is None:
            raise ValueError("solved search results require a solution")
        if self.status is not SearchStatus.SOLVED and self.solution is not None:
            raise ValueError("unsolved search results cannot contain a solution")

    @property
    def is_solved(self) -> bool:
        return self.status is SearchStatus.SOLVED


StateCallback: TypeAlias = Callable[[State], None]
TransitionCallback: TypeAlias = Callable[[Transition], None]
DepthCallback: TypeAlias = Callable[[int], None]
ValueCallback: TypeAlias = Callable[[float], None]


class _CallbackErrors:
    def __init__(self) -> None:
        self.error: BaseException | None = None
        self.traceback: TracebackType | None = None

    def capture(self, error: BaseException) -> None:
        if self.error is None:
            self.error = error
            self.traceback = error.__traceback__

    def raise_if_set(self) -> None:
        if self.error is not None:
            raise self.error.with_traceback(self.traceback)


def _state_callback(
    callback: StateCallback | None,
    problem: Problem,
    errors: _CallbackErrors,
    keepalive: list[object],
) -> ctypes.c_void_p:
    if callback is None:
        return ctypes.c_void_p()

    def trampoline(handle: int) -> int:
        if errors.error is not None:
            return 0
        try:
            callback(State._from_handle(handle, problem))
            return 1
        except BaseException as error:
            errors.capture(error)
            return 0

    native_callback = STATE_CALLBACK(trampoline)
    keepalive.append(native_callback)
    return ctypes.cast(native_callback, ctypes.c_void_p)


def _transition_callback(
    callback: TransitionCallback | None,
    problem: Problem,
    errors: _CallbackErrors,
    keepalive: list[object],
) -> ctypes.c_void_p:
    if callback is None:
        return ctypes.c_void_p()

    def trampoline(source: int, action: int, target: int) -> int:
        if errors.error is not None:
            return 0
        try:
            callback(Transition(
                State._from_handle(source, problem),
                GroundAction._from_handle(action, problem),
                State._from_handle(target, problem),
            ))
            return 1
        except BaseException as error:
            errors.capture(error)
            return 0

    native_callback = TRANSITION_CALLBACK(trampoline)
    keepalive.append(native_callback)
    return ctypes.cast(native_callback, ctypes.c_void_p)


def _depth_callback(
    callback: DepthCallback | None,
    errors: _CallbackErrors,
    keepalive: list[object],
) -> ctypes.c_void_p:
    if callback is None:
        return ctypes.c_void_p()

    def trampoline(value: float) -> int:
        if errors.error is not None:
            return 0
        try:
            callback(int(value))
            return 1
        except BaseException as error:
            errors.capture(error)
            return 0

    native_callback = DOUBLE_CALLBACK(trampoline)
    keepalive.append(native_callback)
    return ctypes.cast(native_callback, ctypes.c_void_p)


def _value_callback(
    callback: ValueCallback | None,
    errors: _CallbackErrors,
    keepalive: list[object],
) -> ctypes.c_void_p:
    if callback is None:
        return ctypes.c_void_p()

    def trampoline(value: float) -> int:
        if errors.error is not None:
            return 0
        try:
            callback(float(value))
            return 1
        except BaseException as error:
            errors.capture(error)
            return 0

    native_callback = DOUBLE_CALLBACK(trampoline)
    keepalive.append(native_callback)
    return ctypes.cast(native_callback, ctypes.c_void_p)


def _heuristic_handle(
    heuristic: Heuristic,
    goal: GroundConjunctiveCondition,
    errors: _CallbackErrors,
    keepalive: list[object],
) -> tuple[int, bool]:
    if isinstance(heuristic, PerfectHeuristic):
        return heuristic._handle_for(goal), False
    if isinstance(heuristic, _NativeHeuristic):
        return heuristic._handle, False

    def evaluate(
        state_handle: int,
        result: _Pointer[ctypes.c_double],
    ) -> int:
        if errors.error is not None:
            return 0
        try:
            value = _validate_value(heuristic.evaluate(
                State._from_handle(state_handle, heuristic.problem), goal))
            result[0] = value
            return 1
        except BaseException as error:
            errors.capture(error)
            return 0

    def is_preferred(
        state_handle: int,
        action_handle: int,
        result: _Pointer[ctypes.c_byte],
    ) -> int:
        if errors.error is not None:
            return 0
        try:
            preferred = heuristic.is_preferred(
                State._from_handle(state_handle, heuristic.problem),
                GroundAction._from_handle(action_handle, heuristic.problem),
                goal,
            )
            if not isinstance(preferred, bool):
                raise TypeError("Heuristic.is_preferred() must return bool")
            result[0] = 1 if preferred else 0
            return 1
        except BaseException as error:
            errors.capture(error)
            return 0

    native_evaluate = HEURISTIC_CALLBACK(evaluate)
    native_is_preferred = PREFERRED_CALLBACK(is_preferred)
    keepalive.extend((native_evaluate, native_is_preferred))
    handle = int(lib.mimir_heuristic_callback(
        heuristic.problem._handle,
        ctypes.cast(native_evaluate, ctypes.c_void_p),
        ctypes.cast(native_is_preferred, ctypes.c_void_p),
    ))
    if handle == 0:
        raise_last_error("could not create Python heuristic adapter")
    return handle, True


def _validate_limits(
    timeout_seconds: float | None,
    max_expanded_states: int | None,
) -> tuple[float, int]:
    if timeout_seconds is None:
        native_timeout = -1.0
    else:
        if isinstance(timeout_seconds, bool) or not isinstance(timeout_seconds, (int, float)):
            raise TypeError("timeout_seconds must be a real number or None")
        native_timeout = float(timeout_seconds)
        if not math.isfinite(native_timeout) or native_timeout < 0.0:
            raise ValueError("timeout_seconds must be finite and nonnegative")

    if max_expanded_states is None:
        native_limit = -1
    else:
        if isinstance(max_expanded_states, bool) or not isinstance(max_expanded_states, int):
            raise TypeError("max_expanded_states must be int or None")
        if max_expanded_states < 0:
            raise ValueError("max_expanded_states must be nonnegative")
        native_limit = max_expanded_states
    return native_timeout, native_limit


def _search(
    algorithm: str,
    problem: Problem,
    *,
    heuristic: Heuristic | None,
    start_state: State | None,
    goal: GroundConjunctiveCondition | None,
    timeout_seconds: float | None,
    max_expanded_states: int | None,
    on_expand: StateCallback | None,
    on_goal: StateCallback | None,
    on_generate: TransitionCallback | None,
    on_discover: TransitionCallback | None,
    on_prune: TransitionCallback | None,
    on_finish_depth: DepthCallback | None,
    on_finish_value: ValueCallback | None,
    on_new_best_h: ValueCallback | None,
) -> SearchResult:
    if not isinstance(problem, Problem):
        raise TypeError("problem must be a Problem")
    if start_state is not None and not isinstance(start_state, State):
        raise TypeError("start_state must be a State or None")
    if start_state is not None and start_state.problem != problem:
        raise ValueError("start_state belongs to a different problem")
    if heuristic is not None:
        if not isinstance(heuristic, Heuristic):
            raise TypeError("heuristic must be a Heuristic")
        if heuristic.problem != problem:
            raise ValueError("heuristic belongs to a different problem")
    effective_goal = problem.goal if goal is None else goal
    if not isinstance(effective_goal, GroundConjunctiveCondition):
        raise TypeError("goal must be a GroundConjunctiveCondition")
    if effective_goal.problem != problem:
        raise ValueError("goal belongs to a different problem")
    effective_start = problem.initial_state if start_state is None else start_state
    native_timeout, native_limit = _validate_limits(timeout_seconds, max_expanded_states)

    if effective_goal.holds(effective_start):
        if on_goal is not None:
            on_goal(effective_start)
        return SearchResult(
            SearchStatus.SOLVED,
            Solution((), 0.0, effective_start),
            SearchStatistics(0, 1, 0, 0.0, 0.0),
        )

    errors = _CallbackErrors()
    keepalive: list[object] = []
    callbacks = (
        _state_callback(on_expand, problem, errors, keepalive),
        _state_callback(on_goal, problem, errors, keepalive),
        _transition_callback(on_generate, problem, errors, keepalive),
        _transition_callback(on_discover, problem, errors, keepalive),
        _transition_callback(on_prune, problem, errors, keepalive),
        _depth_callback(on_finish_depth, errors, keepalive)
        if on_finish_depth is not None
        else _value_callback(on_finish_value, errors, keepalive),
        _value_callback(on_new_best_h, errors, keepalive),
    )
    heuristic_handle = 0
    owns_heuristic_handle = False
    if heuristic is not None:
        heuristic_handle, owns_heuristic_handle = _heuristic_handle(
            heuristic, effective_goal, errors, keepalive)

    try:
        try:
            result_handle = int(lib.mimir_search(
                algorithm.encode("utf-8"),
                problem._handle,
                heuristic_handle,
                effective_start._handle,
                effective_goal._handle,
                native_timeout,
                native_limit,
                *callbacks,
            ))
        except MimirError:
            errors.raise_if_set()
            raise
        errors.raise_if_set()
        if result_handle == 0:
            raise_last_error("native search failed")
        return _build_result(problem, effective_start, result_handle)
    finally:
        if owns_heuristic_handle:
            free_handle(heuristic_handle)


def _build_result(problem: Problem, start: State, handle: int) -> SearchResult:
    try:
        status_code = int(lib.mimir_plan_result_get_status(handle))
        statistics = SearchStatistics(
            int(lib.mimir_plan_result_get_nodes_expanded(handle)),
            int(lib.mimir_plan_result_get_nodes_generated(handle)),
            int(lib.mimir_plan_result_get_max_depth(handle)),
            float(lib.mimir_plan_result_get_setup_time_ms(handle)) / 1000.0,
            float(lib.mimir_plan_result_get_search_time_ms(handle)) / 1000.0,
        )
        status = {
            0: SearchStatus.EXHAUSTED,
            1: SearchStatus.SOLVED,
            2: SearchStatus.TIMED_OUT,
            3: SearchStatus.EXPANSION_LIMIT_REACHED,
        }.get(status_code)
        if status is None:
            raise RuntimeError(f"unknown native search status {status_code}")
        if status is not SearchStatus.SOLVED:
            return SearchResult(status, None, statistics)

        actions: list[GroundAction] = []
        for index in range(int(lib.mimir_plan_result_get_plan_length(handle))):
            action_handle = int(lib.mimir_plan_result_get_ground_action(handle, index))
            if action_handle == 0:
                raise RuntimeError("native result contained a null plan action")
            actions.append(GroundAction._from_handle(action_handle, problem))
        goal_state = start
        for action in actions:
            goal_state = action.apply(goal_state)
        return SearchResult(
            status,
            Solution(
                tuple(actions),
                float(lib.mimir_plan_result_get_plan_cost(handle)),
                goal_state,
            ),
            statistics,
        )
    finally:
        free_handle(handle)


def bfs(
    problem: Problem,
    *,
    start_state: State | None = None,
    goal: GroundConjunctiveCondition | None = None,
    timeout_seconds: float | None = None,
    max_expanded_states: int | None = None,
    on_expand: StateCallback | None = None,
    on_goal: StateCallback | None = None,
    on_generate: TransitionCallback | None = None,
    on_discover: TransitionCallback | None = None,
    on_prune: TransitionCallback | None = None,
    on_finish_depth: DepthCallback | None = None,
) -> SearchResult:
    return _search("bfs", problem, heuristic=None, start_state=start_state, goal=goal,
        timeout_seconds=timeout_seconds,
        max_expanded_states=max_expanded_states, on_expand=on_expand, on_goal=on_goal,
        on_generate=on_generate, on_discover=on_discover, on_prune=on_prune,
        on_finish_depth=on_finish_depth, on_finish_value=None, on_new_best_h=None)


def ucs(
    problem: Problem,
    *,
    start_state: State | None = None,
    goal: GroundConjunctiveCondition | None = None,
    timeout_seconds: float | None = None,
    max_expanded_states: int | None = None,
    on_expand: StateCallback | None = None,
    on_goal: StateCallback | None = None,
    on_generate: TransitionCallback | None = None,
    on_discover: TransitionCallback | None = None,
    on_prune: TransitionCallback | None = None,
) -> SearchResult:
    return _search("ucs", problem, heuristic=None, start_state=start_state, goal=goal,
        timeout_seconds=timeout_seconds,
        max_expanded_states=max_expanded_states, on_expand=on_expand, on_goal=on_goal,
        on_generate=on_generate, on_discover=on_discover, on_prune=on_prune,
        on_finish_depth=None, on_finish_value=None, on_new_best_h=None)


def astar(
    problem: Problem,
    heuristic: Heuristic,
    *,
    start_state: State | None = None,
    goal: GroundConjunctiveCondition | None = None,
    timeout_seconds: float | None = None,
    max_expanded_states: int | None = None,
    on_expand: StateCallback | None = None,
    on_goal: StateCallback | None = None,
    on_generate: TransitionCallback | None = None,
    on_discover: TransitionCallback | None = None,
    on_prune: TransitionCallback | None = None,
    on_finish_f_layer: ValueCallback | None = None,
) -> SearchResult:
    return _search("astar", problem, heuristic=heuristic, start_state=start_state, goal=goal,
        timeout_seconds=timeout_seconds,
        max_expanded_states=max_expanded_states, on_expand=on_expand, on_goal=on_goal,
        on_generate=on_generate, on_discover=on_discover, on_prune=on_prune,
        on_finish_depth=None, on_finish_value=on_finish_f_layer, on_new_best_h=None)


def gbfs(
    problem: Problem,
    heuristic: Heuristic,
    *,
    start_state: State | None = None,
    goal: GroundConjunctiveCondition | None = None,
    timeout_seconds: float | None = None,
    max_expanded_states: int | None = None,
    on_expand: StateCallback | None = None,
    on_goal: StateCallback | None = None,
    on_generate: TransitionCallback | None = None,
    on_discover: TransitionCallback | None = None,
    on_prune: TransitionCallback | None = None,
    on_new_best_h: ValueCallback | None = None,
) -> SearchResult:
    return _search("gbfs", problem, heuristic=heuristic, start_state=start_state, goal=goal,
        timeout_seconds=timeout_seconds,
        max_expanded_states=max_expanded_states, on_expand=on_expand, on_goal=on_goal,
        on_generate=on_generate, on_discover=on_discover, on_prune=on_prune,
        on_finish_depth=None, on_finish_value=None, on_new_best_h=on_new_best_h)


def iw(
    problem: Problem,
    *,
    max_width: int = 1,
    start_state: State | None = None,
    goal: GroundConjunctiveCondition | None = None,
    timeout_seconds: float | None = None,
    max_expanded_states: int | None = None,
    on_expand: StateCallback | None = None,
    on_goal: StateCallback | None = None,
    on_generate: TransitionCallback | None = None,
    on_discover: TransitionCallback | None = None,
) -> SearchResult:
    if isinstance(max_width, bool) or not isinstance(max_width, int):
        raise TypeError("max_width must be int")
    if max_width < 0 or max_width > 3:
        raise ValueError("max_width must be in 0..3")
    return _search(f"iw{max_width}", problem, heuristic=None, start_state=start_state, goal=goal,
        timeout_seconds=timeout_seconds,
        max_expanded_states=max_expanded_states, on_expand=on_expand, on_goal=on_goal,
        on_generate=on_generate, on_discover=on_discover, on_prune=None,
        on_finish_depth=None, on_finish_value=None, on_new_best_h=None)


__all__ = [
    "Solution", "SearchResult", "SearchStatistics", "SearchStatus", "Transition",
    "bfs", "ucs", "astar", "gbfs", "iw",
]
