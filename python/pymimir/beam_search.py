"""Native beam search guided by state heuristics or action-aligned Q-values."""

from __future__ import annotations

from typing import Callable

from .advanced import lib
from .heuristics import Heuristic, QHeuristic
from .model import GroundConjunctiveCondition, Problem, State
from .q_search import _run_search, _validate_integer
from .search import SearchResult


def beam(
    problem: Problem,
    heuristic: Heuristic,
    *,
    beam_size: int = 1,
    max_depth: int = 2**31 - 1,
    start_state: State | None = None,
    goal: GroundConjunctiveCondition | None = None,
    timeout_seconds: float | None = None,
    max_expanded_states: int | None = None,
    should_stop: Callable[[int], bool] | None = None,
) -> SearchResult:
    """Retain the lowest-valued distinct successors at each depth, with FIFO ties.

    Only retained states become closed. Width-pruned states can be reached later.
    Positive infinity denotes a dead end; other scores must be nonnegative and
    not NaN. Native heuristics are evaluated entirely in C#. Python heuristics
    receive unique candidate states through their ``evaluate_batch`` override;
    its default implementation calls ``evaluate`` once per state.

    ``max_depth`` limits path length; zero permits only an initial goal.
    ``should_stop`` receives the expansion count after each evaluated layer.
    Goal successors return before evaluation, with None for the final score.
    Limits and exhaustion return the best current beam path in ``partial_plan``.
    """
    if not isinstance(heuristic, Heuristic):
        raise TypeError("heuristic must be a Heuristic")
    _validate_integer("beam_size", beam_size, minimum=1)
    _validate_integer("max_depth", max_depth, minimum=0)
    return _run_search(
        problem, heuristic, native_search=lib.mimir_beam_search,
        search_options=(beam_size, max_depth), start_state=start_state, goal=goal,
        timeout_seconds=timeout_seconds, max_expanded_states=max_expanded_states,
        should_stop=should_stop,
    )


def qbeam(
    problem: Problem,
    heuristic: QHeuristic,
    *,
    beam_size: int = 1,
    max_depth: int = 2**31 - 1,
    maximize: bool = True,
    start_state: State | None = None,
    goal: GroundConjunctiveCondition | None = None,
    timeout_seconds: float | None = None,
    max_expanded_states: int | None = None,
    should_stop: Callable[[int], bool] | None = None,
) -> SearchResult:
    """Rank distinct successors by incoming Q-value at each depth, with FIFO ties.

    Each evaluated parent retains its complete ordered successor row, including
    duplicates and closed states. All Q-values must be finite. Higher is better
    unless ``maximize=False``. Width one is greedy rollout. Closure, depth limits,
    stop callbacks, early goals and partial paths follow :func:`beam`.
    """
    if not isinstance(heuristic, QHeuristic):
        raise TypeError("heuristic must be a QHeuristic")
    if not isinstance(maximize, bool):
        raise TypeError("maximize must be bool")
    _validate_integer("beam_size", beam_size, minimum=1)
    _validate_integer("max_depth", max_depth, minimum=0)
    return _run_search(
        problem, heuristic, native_search=lib.mimir_qbeam_search,
        search_options=(beam_size, max_depth, maximize), start_state=start_state, goal=goal,
        timeout_seconds=timeout_seconds, max_expanded_states=max_expanded_states,
        should_stop=should_stop,
    )
