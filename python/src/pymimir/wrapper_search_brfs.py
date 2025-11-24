from typing import Callable, Union

from pymimir.advanced.formalism import GroundAction as AdvancedGroundAction
from pymimir.advanced.search import State as AdvancedState

from pymimir.advanced.search import BrFSOptions as AdvancedBrFSOptions
# from pymimir.advanced.search import BrFSStatistics as AdvancedBrFSStatistics
from pymimir.advanced.search import find_solution_brfs as advanced_brfs
from pymimir.advanced.search import IBrFSEventHandler as AdvancedBrFSEventHandler

from .wrapper_formalism import GroundAction, Problem, State
from .wrapper_search import SearchResult


# -----------------
# Search algorithms
# -----------------

def brfs(problem: 'Problem',
         start_state: 'State',
         max_time_seconds: float = -1,
         max_num_states: int = -1,
         on_expand_state: 'Union[Callable[[State], None], None]' = None,
         on_expand_goal_state: 'Union[Callable[[State], None], None]' = None,
         on_generate_state: 'Union[Callable[[State, GroundAction, float, State], None], None]' = None,
         on_generate_new_state: 'Union[Callable[[State, GroundAction, float, State], None], None]' = None,
         on_prune_state: 'Union[Callable[[State, GroundAction, float, State], None], None]' = None,
         on_finish_g_layer: 'Union[Callable[[float], None], None]' = None) -> 'SearchResult':
    """
    Breadth-First Search (BrFS) search algorithm.

    :param problem: The problem to solve.
    :type problem: Problem
    :param start_state: The initial state from which to start the search.
    :type start_state: State
    :param max_time_seconds: Maximum time allowed for the search in seconds. Default is -1 (no limit).
    :type max_time_seconds: float
    :param max_num_states: Maximum number of states to explore. Default is -1 (no limit).
    :type max_num_states: int
    :param on_expand_state: Callback function called when a state is expanded.
    :type on_expand_state: Callable[[State], None]
    :param on_expand_goal_state: Callback function called when a goal state is expanded.
    :type on_expand_goal_state: Callable[[State], None]
    :param on_generate_state: Callback function called when a state is generated.
    :type on_generate_state: Callable[[State, GroundAction, float, State], None]
    :param on_generate_new_state: Callback function called when a new state is generated that has not been seen before.
    :type on_generate_new_state: Callable[[State, GroundAction, float, State], None]
    :param on_prune_state: Callback function called when a state is pruned.
    :type on_prune_state: Callable[[State, GroundAction, float, State], None]
    :param on_finish_g_layer: Callback function called when a layer of states is finished.
    :type on_finish_g_layer: Callable[[float], None]
    :return: A SearchResult object containing the status, solution, solution cost, and goal state.
    :rtype: SearchResult
    """
    assert isinstance(problem, Problem), "Problem must be an instance of Problem."
    assert isinstance(start_state, State), "Start state must be an instance of State."
    assert isinstance(max_time_seconds, (int, float)), "max_time_seconds must be an int or float."
    assert isinstance(max_num_states, int), "max_num_states must be an int."
    # Define the event handler with the provided callback functions.
    class EventHandler(AdvancedBrFSEventHandler):
        def __init__(self) -> None:
            super().__init__()

        def on_expand_goal_state(self, advanced_state: 'AdvancedState'):
            nonlocal problem, on_expand_goal_state
            if on_expand_goal_state:
                state = State(advanced_state, problem)
                on_expand_goal_state(state)

        def on_expand_state(self, advanced_state: 'AdvancedState'):
            nonlocal problem, on_expand_state
            if on_expand_state:
                state = State(advanced_state, problem)
                on_expand_state(state)

        def on_finish_g_layer(self, arg):
            nonlocal problem, on_finish_g_layer
            if on_finish_g_layer:
                on_finish_g_layer(arg)

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
        def on_close_state(self, arg0): pass
        def on_end_search(self, arg0, arg1, arg2, arg3, arg4, arg5): pass
        def on_exhausted(self): pass
        def on_generate_state_not_relaxed(self, arg0, arg1, arg2, arg3): pass
        def on_generate_state_relaxed(self, arg0, arg1, arg2, arg3): pass
        def on_solved(self, arg): pass
        def on_start_search(self, arg0): pass
        def on_unsolvable(self): pass

    # Create options for the BrFS search
    advanced_options = AdvancedBrFSOptions()
    if max_time_seconds > 0: advanced_options.max_time_in_ms = int(max_time_seconds * 1000)
    if max_num_states > 0: advanced_options.max_num_states = max_num_states
    advanced_options.start_state = start_state._advanced_state
    advanced_options.event_handler = EventHandler()
    # Invoke the BrFS search algorithm
    result = advanced_brfs(problem._search_context, advanced_options)
    status = result.status.name.lower()
    solution = [GroundAction(x, problem) for x in result.plan.get_actions()] if result.plan else None
    solution_cost = result.plan.get_cost() if result.plan else None
    goal_state = State(result.goal_state, problem) if result.goal_state else None
    return SearchResult(status, solution, solution_cost, goal_state)
