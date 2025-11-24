from typing import Callable, Union

from pymimir.advanced.formalism import GroundAction as AdvancedGroundAction
from pymimir.advanced.search import State as AdvancedState

from pymimir.advanced.search import BrFSStatistics as AdvancedBrFSStatistics
from pymimir.advanced.search import find_solution_iw as advanced_iw
# from pymimir.advanced.search import find_solution_siw as advanced_siw
from pymimir.advanced.search import IBrFSEventHandler as AdvancedBrFSEventHandler
from pymimir.advanced.search import IWOptions as AdvancedIWOptions
# from pymimir.advanced.search import IWStatistics as AdvancedIWStatistics
# from pymimir.advanced.search import SIWOptions as AdvancedSIWOptions
# from pymimir.advanced.search import SIWStatistics as AdvancedSIWStatistics

from .wrapper_formalism import GroundAction, Problem, State
from .wrapper_search import SearchResult


# ----------------------
# Width-based algorithms
# ----------------------

def iw(problem: 'Problem',
       start_state: 'State',
       max_arity: int,
       on_expand_state: 'Union[Callable[[State], None], None]' = None,
       on_expand_goal_state: 'Union[Callable[[State], None], None]' = None,
       on_generate_state: 'Union[Callable[[State, GroundAction, float, State], None], None]' = None,
       on_generate_new_state: 'Union[Callable[[State, GroundAction, float, State], None], None]' = None,
       on_prune_state: 'Union[Callable[[State, GroundAction, float, State], None], None]' = None) -> 'SearchResult':
    assert isinstance(problem, Problem), "Problem must be an instance of Problem."
    assert isinstance(start_state, State), "Start state must be an instance of State."
    assert isinstance(max_arity, int), "Max arity must be an integer."
    assert max_arity > 0, "Max arity must be positive."
    # Define the event handler with the provided callback functions.
    class EventHandler(AdvancedBrFSEventHandler):
        def __init__(self) -> None:
            super().__init__()

        def on_expand_state(self, advanced_state: 'AdvancedState'):
            nonlocal problem, on_expand_state
            if on_expand_state:
                state = State(advanced_state, problem)
                on_expand_state(state)

        def on_expand_goal_state(self, advanced_state: 'AdvancedState'):
            nonlocal problem, on_expand_goal_state
            if on_expand_goal_state:
                state = State(advanced_state, problem)
                on_expand_goal_state(state)

        def on_generate_state(self, advanced_state: 'AdvancedState', advanced_action: 'AdvancedGroundAction', action_cost: float, advanced_successor_state: 'AdvancedState'):
            nonlocal problem, on_generate_state
            if on_generate_state:
                state = State(advanced_state, problem)
                action = GroundAction(advanced_action, problem)
                successor_state = State(advanced_successor_state, problem)
                on_generate_state(state, action, action_cost, successor_state)

        def on_generate_state_in_search_tree(self, advanced_state: 'AdvancedState', advanced_action: 'AdvancedGroundAction', action_cost: float, advanced_successor_state: 'AdvancedState'):
            nonlocal problem, on_generate_new_state
            if on_generate_new_state:
                state = State(advanced_state, problem)
                action = GroundAction(advanced_action, problem)
                successor_state = State(advanced_successor_state, problem)
                on_generate_new_state(state, action, action_cost, successor_state)

        def on_generate_state_not_in_search_tree(self, advanced_state: 'AdvancedState', advanced_action: 'AdvancedGroundAction', action_cost: float, advanced_successor_state: 'AdvancedState'):
            nonlocal problem, on_prune_state
            if on_prune_state:
                state = State(advanced_state, problem)
                action = GroundAction(advanced_action, problem)
                successor_state = State(advanced_successor_state, problem)
                on_prune_state(state, action, action_cost, successor_state)

        # The following events are ignored in this interface.
        def on_finish_g_layer(self, value: int): pass
        def on_start_search(self, arg: 'AdvancedState'): pass
        def on_end_search(self, arg0: int, arg1: int, arg2: int, arg3: int, arg4: int, arg5: int): pass
        def on_solved(self, arg): pass
        def on_unsolvable(self): pass
        def on_exhausted(self): pass
        def get_statistics(self) -> 'AdvancedBrFSStatistics':
            return AdvancedBrFSStatistics()

    advanced_options = AdvancedIWOptions()
    advanced_options.start_state = start_state._advanced_state
    advanced_options.brfs_event_handler = EventHandler()
    advanced_options.max_arity = max_arity
    result = advanced_iw(problem._search_context, advanced_options)
    status = result.status.name.lower()
    solution = [GroundAction(x, problem) for x in result.plan.get_actions()] if result.plan else None
    solution_cost = result.plan.get_cost() if result.plan else None
    goal_state = State(result.goal_state, problem) if result.goal_state else None
    return SearchResult(status, solution, solution_cost, goal_state)
