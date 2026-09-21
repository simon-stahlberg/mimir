"""Native batched searches and their shared callback and result transport."""

from __future__ import annotations

import ctypes
import math
from _ctypes import CFuncPtr, _Pointer
from numbers import Real
from typing import Callable

from .advanced import free_handle, lib
from .advanced._native import raise_last_error
from .advanced.heuristics import QHEURISTIC_CALLBACK
from .advanced.search import STOP_CALLBACK
from .heuristics import Heuristic, QHeuristic, Successors
from .dead_ends import DeadEndDetector, _detector_handle
from .model import GroundAction, GroundConjunctiveCondition, Problem, State
from .search import SearchResult, SearchStatistics, SearchStatus, Solution, _CallbackErrors, _heuristic_handle, _validate_limits


def qgbfs(
    problem: Problem,
    heuristic: QHeuristic,
    *,
    start_state: State | None = None,
    goal: GroundConjunctiveCondition | None = None,
    dead_end_detector: DeadEndDetector | None = None,
    maximize: bool = True,
    batch_target: int = 1,
    timeout_seconds: float | None = None,
    max_expanded_states: int | None = None,
    should_stop: Callable[[int], bool] | None = None,
) -> SearchResult:
    """Score whole expansions and improve priorities of states still in the frontier.

    ``batch_target`` counts newly reached states, not parents or model calls.
    ``should_stop`` receives the expansion count after each search batch.
    Scores must be finite. Equal scores retain the earlier path and FIFO order.
    Goal successors return immediately, with None for the final action score.
    """
    if not isinstance(heuristic, QHeuristic):
        raise TypeError("heuristic must be a QHeuristic")
    if not isinstance(maximize, bool):
        raise TypeError("maximize must be bool")
    _validate_integer("batch_target", batch_target, minimum=1)
    return _run_search(
        problem, heuristic, native_search=lib.mimir_qgbfs_search,
        search_options=(batch_target, maximize), start_state=start_state, goal=goal, dead_end_detector=dead_end_detector,
        timeout_seconds=timeout_seconds, max_expanded_states=max_expanded_states,
        should_stop=should_stop,
    )


def _validate_integer(name: str, value: int, *, minimum: int) -> None:
    if isinstance(value, bool) or not isinstance(value, int):
        raise TypeError(f"{name} must be int")
    if not minimum <= value <= 2**31 - 1:
        raise ValueError(f"{name} must be between {minimum} and {2**31 - 1}")


def _run_search(
    problem: Problem,
    heuristic: Heuristic | QHeuristic,
    *,
    native_search: Callable[..., int],
    search_options: tuple[int, ...],
    start_state: State | None,
    goal: GroundConjunctiveCondition | None,
    dead_end_detector: DeadEndDetector | None,
    timeout_seconds: float | None,
    max_expanded_states: int | None,
    should_stop: Callable[[int], bool] | None,
) -> SearchResult:
    if not isinstance(problem, Problem):
        raise TypeError("problem must be a Problem")
    if heuristic.problem is not problem and heuristic.problem != problem:
        raise ValueError("heuristic belongs to a different problem")
    if should_stop is not None and not callable(should_stop):
        raise TypeError("should_stop must be callable or None")
    start = problem.initial_state if start_state is None else start_state
    effective_goal = problem.goal if goal is None else goal
    if not isinstance(start, State):
        raise TypeError("start_state must be a State")
    if not isinstance(effective_goal, GroundConjunctiveCondition):
        raise TypeError("goal must be a GroundConjunctiveCondition")
    if start.problem is not problem and start.problem != problem:
        raise ValueError("start_state belongs to a different problem")
    if effective_goal.problem is not problem and effective_goal.problem != problem:
        raise ValueError("goal belongs to a different problem")
    detector_handle = _detector_handle(dead_end_detector, problem)
    native_timeout, native_limit = _validate_limits(timeout_seconds, max_expanded_states)
    if max_expanded_states is not None:
        _validate_integer("max_expanded_states", max_expanded_states, minimum=0)
    if lib.mimir_goal_is_satisfied(effective_goal._handle, start._handle):
        return SearchResult(
            SearchStatus.SOLVED,
            Solution((), 0.0, start),
            SearchStatistics(0, 1, 0, 0.0, 0.0, 1, 0, 0),
        )
    errors = _CallbackErrors()
    stopped = False

    def stop(expanded: int) -> int:
        nonlocal stopped
        try:
            assert should_stop is not None
            stopped = should_stop(expanded)
            if not isinstance(stopped, bool):
                raise TypeError("should_stop must return bool")
            return int(stopped)
        except BaseException as error:
            errors.capture(error)
            return 2

    keepalive: list[object] = []
    owned = False
    native_heuristic: int | ctypes.c_void_p
    if isinstance(heuristic, QHeuristic):
        native_evaluate = _qheuristic_callback(problem, heuristic, effective_goal, errors)
        keepalive.append(native_evaluate)
        native_heuristic = ctypes.cast(native_evaluate, ctypes.c_void_p)
    else:
        native_heuristic, owned = _heuristic_handle(heuristic, effective_goal, errors, keepalive)
    native_stop = STOP_CALLBACK(stop) if should_stop is not None else None
    result = 0
    try:
        result = int(native_search(
            problem._handle, start._handle, effective_goal._handle,
            native_heuristic, *search_options, native_timeout, native_limit,
            ctypes.cast(native_stop, ctypes.c_void_p) if native_stop is not None else None,
            detector_handle,
        ))
        errors.raise_if_set()
        if result == 0:
            raise_last_error("native batched search failed")
    except BaseException:
        free_handle(result)
        errors.raise_if_set()
        raise
    finally:
        if owned:
            assert isinstance(native_heuristic, int)
            free_handle(native_heuristic)
    return _build_result(problem, result, stopped)


def _qheuristic_callback(
    problem: Problem, heuristic: QHeuristic, effective_goal: GroundConjunctiveCondition,
    errors: _CallbackErrors,
) -> CFuncPtr:
    def evaluate(
        handles: _Pointer[ctypes.c_int],
        offsets: _Pointer[ctypes.c_int],
        count: int,
        output: _Pointer[ctypes.c_double],
    ) -> int:
        if errors.error is not None:
            return 0
        try:
            expansions: list[tuple[State, Successors]] = []
            for row in range(count):
                handle = handles[row]
                handles[row] = 0
                state = State._from_handle(handle, problem)
                successors = []
                for index in range(offsets[row], offsets[row + 1]):
                    position = count + 2 * index
                    handle = handles[position]
                    handles[position] = 0
                    action = GroundAction._from_handle(handle, problem)
                    handle = handles[position + 1]
                    handles[position + 1] = 0
                    successor = State._from_handle(handle, problem)
                    successors.append((action, successor))
                state._applicable_actions = tuple(action for action, _ in successors)
                expansions.append((state, tuple(successors)))
            values = heuristic.evaluate_batch(expansions, effective_goal)
            if len(values) != count:
                raise ValueError("Q-heuristic returned an incorrect number of rows")
            for row, scores in enumerate(values):
                if len(scores) != offsets[row + 1] - offsets[row]:
                    raise ValueError("Q-heuristic score count must match the number of successors")
                for index, value in enumerate(scores, offsets[row]):
                    if isinstance(value, bool) or not isinstance(value, Real):
                        raise TypeError("Q-heuristic scores must be real numbers")
                    numeric_value = float(value)
                    if not math.isfinite(numeric_value):
                        raise ValueError(
                            f"Q-heuristic score at row {row}, action {index - offsets[row]} must be finite"
                        )
                    output[index] = numeric_value
            return 1
        except BaseException as error:
            errors.capture(error)
            return 0

    return QHEURISTIC_CALLBACK(evaluate)


def _build_result(problem: Problem, handle: int, stopped: bool) -> SearchResult:
    try:
        status_code = int(lib.mimir_plan_result_get_status(handle))
        status = {
            0: SearchStatus.EXHAUSTED,
            1: SearchStatus.SOLVED,
            2: SearchStatus.STOPPED if stopped else SearchStatus.TIMED_OUT,
            3: SearchStatus.EXPANSION_LIMIT_REACHED,
            4: SearchStatus.DEPTH_LIMIT_REACHED,
            5: SearchStatus.DEAD_END,
        }[status_code]
        stats = SearchStatistics(
            int(lib.mimir_plan_result_get_nodes_expanded(handle)),
            int(lib.mimir_plan_result_get_nodes_generated(handle)),
            int(lib.mimir_plan_result_get_max_depth(handle)),
            float(lib.mimir_plan_result_get_setup_time_ms(handle)) / 1000,
            float(lib.mimir_plan_result_get_search_time_ms(handle)) / 1000,
            int(lib.mimir_plan_result_get_visited_states(handle)),
            int(lib.mimir_plan_result_get_generated_transitions(handle)),
            int(lib.mimir_plan_result_get_evaluated_candidates(handle)),
        )
        count = int(lib.mimir_plan_result_get_plan_length(handle) if status is SearchStatus.SOLVED
                    else lib.mimir_plan_result_get_partial_plan_length(handle))
        handles = (ctypes.c_int * (count + 1))()
        values = (ctypes.c_double * count)()
        has_values = (ctypes.c_ubyte * count)()
        try:
            lib.mimir_plan_result_copy_scored_path(handle, handles, values, has_values, count)
            actions = []
            for index in range(count):
                action_handle = handles[index]
                handles[index] = 0
                actions.append(GroundAction._from_handle(action_handle, problem))
            solution = None
            if status is SearchStatus.SOLVED:
                state_handle = handles[count]
                handles[count] = 0
                solution = Solution(tuple(actions), float(lib.mimir_plan_result_get_plan_cost(handle)),
                                    State._from_handle(state_handle, problem))
            return SearchResult(status, solution, stats,
                                () if solution is not None else tuple(actions),
                                tuple(value if present else None for value, present in zip(values, has_values)))
        finally:
            for owned in handles:
                free_handle(owned)
    finally:
        free_handle(handle)
