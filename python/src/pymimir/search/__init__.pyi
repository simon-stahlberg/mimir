from enum import Enum
from typing import Union, List, MutableSet, Tuple, Iterator

from pymimir.formalism import Problem, PDDLFactories


class SearchNodeStatus(Enum):
    NEW: 'SearchNodeStatus'
    OPEN: 'SearchNodeStatus'
    CLOSED: 'SearchNodeStatus'
    DEAD_END: 'SearchNodeStatus'

class SearchStatus(Enum):
    IN_PROGRESS: 'SearchStatus'
    OUT_OF_TIME: 'SearchStatus'
    OUT_OF_MEMORY: 'SearchStatus'
    FAILED: 'SearchStatus'
    EXHAUSTED: 'SearchStatus'
    SOLVED: 'SearchStatus'

class State:
    def __hash__(self) -> int: ...
    def __eq__(self, other : "State") -> bool: ...
    def __iter__(self) -> Iterator[int]: ...
    def to_string(self, pddl_factories: PDDLFactories) -> str: ...

class GroundAction:
    def __hash__(self) -> int: ...
    def __eq__(self, other : "GroundAction") -> bool: ...
    def to_string(self, pddl_factories: PDDLFactories) -> str: ...

class IAAG:
    def generate_applicable_actions(self, state: State) -> List[GroundAction]: ...
    def get_action(self, action_id: int) -> GroundAction: ...

class LiftedAAG(IAAG):
    def __init__(self, problem: Problem, pddl_factories: PDDLFactories) -> None: ...

class GroundedAAG(IAAG):
    def __init__(self, problem: Problem, pddl_factories: PDDLFactories) -> None: ...

class ISSG:
    def get_or_create_initial_state(self, problem: Problem) -> State: ...
    def get_or_create_successor_state(self, state: State, action: GroundAction) -> State: ...
    def get_state_count(self) -> int: ...

class SSG(ISSG):
    def __init__(self, pddl_factories: PDDLFactories, aag: IAAG) -> None: ...

class IHeuristic:
    def compute_heuristic(self, state: State) -> float: ...

class BlindHeuristic(IHeuristic):
    def __init__(self, problem: Problem) -> None: ...

class IEventHandler:
    pass

class MinimalEventHandler(IEventHandler):
    pass

class DebugEventHandler(IEventHandler):
    pass

class IAlgorithm:
    def find_solution(self) -> Tuple[SearchStatus, List[str]]: ...

class BrFsAlgorithm(IAlgorithm):
    def __init__(self, problem: Problem, pddl_factories: PDDLFactories, state_repository: ISSG, successor_state_generator: IAAG, event_handler: IEventHandler) -> None: ...

class AStarAlgorithm(IAlgorithm):
    def __init__(self, problem: Problem, pddl_factories: PDDLFactories, state_repository: ISSG, successor_state_generator: IAAG, heuristic: IHeuristic, event_handler: IEventHandler) -> None: ...
