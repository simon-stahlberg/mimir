from typing import Callable, Union

from pymimir.advanced.formalism import GroundAction as AdvancedGroundAction
from pymimir.advanced.search import IHeuristic as AdvancedHeuristicBase
from pymimir.advanced.search import State as AdvancedState

from pymimir.advanced.search import find_solution_gbfs_eager as advanced_gbfs_eager
from pymimir.advanced.search import find_solution_gbfs_lazy as advanced_gbfs_lazy
from pymimir.advanced.search import GBFSEagerOptions as AdvancedGBFSEagerOptions
from pymimir.advanced.search import GBFSLazyOptions as AdvancedGBFSLazyOptions
from pymimir.advanced.search import IGBFSEagerEventHandler as AdvancedGBFSEagerEventHandler
from pymimir.advanced.search import IGBFSLazyEventHandler as AdvancedGBFSLazyEventHandler

from .wrapper_formalism import GroundAction, Problem, State
from .wrapper_heuristics import Heuristic, AdvancedHeuristicAdapter
from .wrapper_search import SearchResult


# ---------------
# GBFS algorithms
# ---------------

def gbfs_eager(problem: 'Problem',
               start_state: 'State',
               heuristic: 'Heuristic',
               max_time_seconds: float = -1,
               max_num_states: int = -1,
               on_expand_state: 'Union[Callable[[State], None], None]' = None,
               on_expand_goal_state: 'Union[Callable[[State], None], None]' = None,
               on_generate_state: 'Union[Callable[[State, GroundAction, float, State], None], None]' = None,
               on_prune_state: 'Union[Callable[[State], None], None]' = None,
               on_new_best_h_value: 'Union[Callable[[float], None], None]' = None) -> 'SearchResult':
    """
    Greedy Best-First Search (GBFS) search algorithm with eager evaluation.

    :param problem: The problem to solve.
    :type problem: Problem
    :param start_state: The initial state from which to start the search.
    :type start_state: State
    :param heuristic: The heuristic function to use for the search.
    :type heuristic: Heuristic
    :param max_time_seconds: Maximum time allowed for the search in seconds. Default is -1 (no limit).
    :type max_time_seconds: float
    :param max_num_states: Maximum number of states to explore. Default is -1 (no limit).
    :type max_num_states: int
    :param on_expand_state: Callback function called when a state is expanded.
    :type on_expand_state: Callable[[State], None]
    :param on_expand_goal_state: Callback function called when a goal state is expanded.
    :type on_expand_goal_state: Callable[[State], None]
    :param on_generate_state: Callback function called when a new state is generated.
    :type on_generate_state: Callable[[State, GroundAction, float, State], None]
    :param on_prune_state: Callback function called when a state is pruned.
    :type on_prune_state: Callable[[State], None]
    :param on_new_best_h_value: Callback function called when a layer of states is finished.
    :type on_new_best_h_value: Callable[[float], None]
    :return: A SearchResult object containing the status, solution, solution cost, and goal state.
    :rtype: SearchResult
    """
    assert isinstance(problem, Problem), "Problem must be an instance of Problem."
    assert isinstance(start_state, State), "Start state must be an instance of State."
    assert isinstance(heuristic, Heuristic), "Heuristic must be an instance of Heuristic."
    assert isinstance(max_time_seconds, (int, float)), "max_time_seconds must be an int or float."
    assert isinstance(max_num_states, int), "max_num_states must be an int."
    # Define the event handler with the provided callback functions.
    class EventHandler(AdvancedGBFSEagerEventHandler):
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

        def on_generate_state(self, advanced_state: 'AdvancedState', advanced_action: 'AdvancedGroundAction', action_cost: float, advanced_successor_state: 'AdvancedState'):
            nonlocal problem, on_generate_state
            if on_generate_state:
                state = State(advanced_state, problem)
                action = GroundAction(advanced_action, problem)
                successor_state = State(advanced_successor_state, problem)
                on_generate_state(state, action, action_cost, successor_state)

        def on_prune_state(self, advanced_state: 'AdvancedState'):
            nonlocal problem, on_prune_state
            if on_prune_state:
                state = State(advanced_state, problem)
                on_prune_state(state)

        def on_new_best_h_value(self, h_value: 'float'):
            nonlocal on_new_best_h_value
            if on_new_best_h_value:
                on_new_best_h_value(h_value)

        # The following events are ignored in this interface.
        def on_close_state(self, arg0): pass
        def on_end_search(self, arg0, arg1, arg2, arg3, arg4, arg5): pass
        def on_exhausted(self): pass
        def on_generate_state_not_relaxed(self, arg0, arg1, arg2, arg3): pass
        def on_generate_state_relaxed(self, arg0, arg1, arg2, arg3): pass
        def on_solved(self, arg): pass
        def on_start_search(self, arg0, arg1, arg2): pass
        def on_unsolvable(self): pass

    # Create options for the GBFS search
    advanced_options = AdvancedGBFSEagerOptions()
    if max_time_seconds > 0: advanced_options.max_time_in_ms = int(max_time_seconds * 1000)
    if max_num_states > 0: advanced_options.max_num_states = max_num_states
    advanced_options.start_state = start_state._advanced_state
    advanced_options.event_handler = EventHandler()
    # The GBFS search algorithm expects an advanced heuristic: get the internal one, or create a proxy
    if hasattr(heuristic, '_advanced_heuristic') and isinstance(heuristic._advanced_heuristic, AdvancedHeuristicBase):  # type: ignore
        advanced_heuristic = heuristic._advanced_heuristic  # type: ignore
    elif isinstance(heuristic, Heuristic):
        advanced_heuristic = AdvancedHeuristicAdapter(heuristic, problem)
    else:
        raise TypeError("Heuristic must be an instance of Heuristic.")
    # Invoke the GBFS search algorithm
    result = advanced_gbfs_eager(problem._search_context, advanced_heuristic, advanced_options)
    status = result.status.name.lower()
    solution = [GroundAction(x, problem) for x in result.plan.get_actions()] if result.plan else None
    solution_cost = result.plan.get_cost() if result.plan else None
    goal_state = State(result.goal_state, problem) if result.goal_state else None
    return SearchResult(status, solution, solution_cost, goal_state)


def gbfs_lazy(problem: 'Problem',
              start_state: 'State',
              heuristic: 'Heuristic',
              max_time_seconds: float = -1,
              max_num_states: int = -1,
              on_expand_state: 'Union[Callable[[State], None], None]' = None,
              on_expand_goal_state: 'Union[Callable[[State], None], None]' = None,
              on_generate_state: 'Union[Callable[[State, GroundAction, float, State], None], None]' = None,
              on_prune_state: 'Union[Callable[[State], None], None]' = None,
              on_new_best_h_value: 'Union[Callable[[float], None], None]' = None) -> 'SearchResult':
    """
    Greedy Best-First Search (GBFS) search algorithm with lazy evaluation.

    :param problem: The problem to solve.
    :type problem: Problem
    :param start_state: The initial state from which to start the search.
    :type start_state: State
    :param heuristic: The heuristic function to use for the search.
    :type heuristic: Heuristic
    :param max_time_seconds: Maximum time allowed for the search in seconds. Default is -1 (no limit).
    :type max_time_seconds: float
    :param max_num_states: Maximum number of states to explore. Default is -1 (no limit).
    :type max_num_states: int
    :param on_expand_state: Callback function called when a state is expanded.
    :type on_expand_state: Callable[[State], None]
    :param on_expand_goal_state: Callback function called when a goal state is expanded.
    :type on_expand_goal_state: Callable[[State], None]
    :param on_generate_state: Callback function called when a new state is generated.
    :type on_generate_state: Callable[[State, GroundAction, float, State], None]
    :param on_prune_state: Callback function called when a state is pruned.
    :type on_prune_state: Callable[[State], None]
    :param on_new_best_h_value: Callback function called when a layer of states is finished.
    :type on_new_best_h_value: Callable[[float], None]
    :return: A SearchResult object containing the status, solution, solution cost, and goal state.
    :rtype: SearchResult
    """
    assert isinstance(problem, Problem), "Problem must be an instance of Problem."
    assert isinstance(start_state, State), "Start state must be an instance of State."
    assert isinstance(heuristic, Heuristic), "Heuristic must be an instance of Heuristic."
    assert isinstance(max_time_seconds, (int, float)), "max_time_seconds must be an int or float."
    assert isinstance(max_num_states, int), "max_num_states must be an int."
    # Define the event handler with the provided callback functions.
    class EventHandler(AdvancedGBFSLazyEventHandler):
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

        def on_generate_state(self, advanced_state: 'AdvancedState', advanced_action: 'AdvancedGroundAction', action_cost: float, advanced_successor_state: 'AdvancedState'):
            nonlocal problem, on_generate_state
            if on_generate_state:
                state = State(advanced_state, problem)
                action = GroundAction(advanced_action, problem)
                successor_state = State(advanced_successor_state, problem)
                on_generate_state(state, action, action_cost, successor_state)

        def on_prune_state(self, advanced_state: 'AdvancedState'):
            nonlocal problem, on_prune_state
            if on_prune_state:
                state = State(advanced_state, problem)
                on_prune_state(state)

        def on_new_best_h_value(self, h_value: 'float'):
            nonlocal problem, on_new_best_h_value
            if on_new_best_h_value:
                on_new_best_h_value(h_value)

        # The following events are ignored in this interface.
        def on_close_state(self, arg0): pass
        def on_end_search(self, arg0, arg1, arg2, arg3, arg4, arg5): pass
        def on_exhausted(self): pass
        def on_generate_state_not_relaxed(self, arg0, arg1, arg2, arg3): pass
        def on_generate_state_relaxed(self, arg0, arg1, arg2, arg3): pass
        def on_solved(self, arg): pass
        def on_start_search(self, arg0, arg1, arg2): pass
        def on_unsolvable(self): pass

    # Create options for the GBFS search
    advanced_options = AdvancedGBFSLazyOptions()
    if max_time_seconds > 0: advanced_options.max_time_in_ms = int(max_time_seconds * 1000)
    if max_num_states > 0: advanced_options.max_num_states = max_num_states
    advanced_options.start_state = start_state._advanced_state
    advanced_options.event_handler = EventHandler()
    # The GBFS search algorithm expects an advanced heuristic: get the internal one, or create a proxy
    if hasattr(heuristic, '_advanced_heuristic') and isinstance(heuristic._advanced_heuristic, AdvancedHeuristicBase):  # type: ignore
        advanced_heuristic = heuristic._advanced_heuristic  # type: ignore
    elif isinstance(heuristic, Heuristic):
        advanced_heuristic = AdvancedHeuristicAdapter(heuristic, problem)
    else:
        raise TypeError("Heuristic must be an instance of Heuristic.")
    # Invoke the GBFS search algorithm
    result = advanced_gbfs_lazy(problem._search_context, advanced_heuristic, advanced_options)
    status = result.status.name.lower()
    solution = [GroundAction(x, problem) for x in result.plan.get_actions()] if result.plan else None
    solution_cost = result.plan.get_cost() if result.plan else None
    goal_state = State(result.goal_state, problem) if result.goal_state else None
    return SearchResult(status, solution, solution_cost, goal_state)
