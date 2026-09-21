"""Sound, incomplete dead-end detection for states reachable from the problem's initial state."""

from __future__ import annotations

from collections.abc import Sequence
import ctypes

from .advanced import free_handle, lib
from .advanced._native import _NativeOwner, _create_finalizer, raise_last_error
from .model import GroundConjunctiveCondition, Problem, State


class DeadEndDetector(_NativeOwner):
    """Native detector: True proves a dead end for the supplied goal; False means unknown."""

    def __init__(self, handle: int, problem: Problem | None) -> None:
        if handle == 0:
            raise_last_error("could not create dead-end detector")
        self._handle = handle
        self._problem = problem
        self._finalizer = _create_finalizer(self, free_handle, handle)

    def is_dead_end(self, state: State, goal: GroundConjunctiveCondition) -> bool:
        if not isinstance(state, State):
            raise TypeError("state must be a State")
        if not isinstance(goal, GroundConjunctiveCondition):
            raise TypeError("goal must be a GroundConjunctiveCondition")
        if state.problem != goal.problem:
            raise ValueError("state and goal belong to different problems")
        _detector_handle(self, state.problem)
        result = int(lib.mimir_dead_end_detector_evaluate(self._handle, state._handle, goal._handle))
        if result < 0:
            raise_last_error("dead-end detection failed")
        return bool(result)


class H2DeadEndDetector(DeadEndDetector):
    def __init__(self, problem: Problem) -> None:
        if not isinstance(problem, Problem):
            raise TypeError("problem must be a Problem")
        super().__init__(int(lib.mimir_dead_end_detector_h2(problem._handle)), problem)


class DisjunctiveDeadEndDetector(DeadEndDetector):
    """Evaluate detectors in order until one proves a dead end. An empty list returns False."""

    def __init__(self, detectors: Sequence[DeadEndDetector]) -> None:
        children = tuple(detectors)
        problem = None
        for detector in children:
            if not isinstance(detector, DeadEndDetector):
                raise TypeError("detectors must contain DeadEndDetector instances")
            if detector._problem is not None:
                if problem is not None and problem != detector._problem:
                    raise ValueError("detectors belong to different problems")
                problem = detector._problem
        handles = (ctypes.c_int * len(children))(*(detector._handle for detector in children))
        super().__init__(int(lib.mimir_dead_end_detector_disjunction(handles, len(children))), problem)
        self._detectors = children


def _detector_handle(detector: DeadEndDetector | None, problem: Problem) -> int:
    if detector is None:
        return 0
    if not isinstance(detector, DeadEndDetector):
        raise TypeError("dead_end_detector must be a DeadEndDetector or None")
    if detector._problem is not None and detector._problem != problem:
        raise ValueError("dead-end detector belongs to a different problem")
    return detector._handle
