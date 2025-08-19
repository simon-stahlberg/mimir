from typing import Callable, Union

from pymimir.advanced.formalism import GroundAction as AdvancedGroundAction
from pymimir.advanced.search import AddHeuristic as AdvancedAddHeuristic
from pymimir.advanced.search import BlindHeuristic as AdvancedBlindHeuristic
from pymimir.advanced.search import DeleteRelaxedProblemExplorator as AdvancedDeleteRelaxedProblemExplorator
from pymimir.advanced.search import FFHeuristic as AdvancedFFHeuristic
from pymimir.advanced.search import IHeuristic as AdvancedHeuristicBase
from pymimir.advanced.search import MaxHeuristic as AdvancedMaxHeuristic
from pymimir.advanced.search import PerfectHeuristic as AdvancedPerfectHeuristic
from pymimir.advanced.search import PreferredActions as AdvancedPreferredActions
from pymimir.advanced.search import SetAddHeuristic as AdvancedSetAddHeuristic
from pymimir.advanced.search import State as AdvancedState

from pymimir.advanced.search import find_solution_astar_eager as advanced_astar_eager
from pymimir.advanced.search import find_solution_iw as advanced_iw

from pymimir.advanced.search import AStarEagerOptions as AdvancedAStarEagerOptions
from pymimir.advanced.search import IWOptions as AdvancedIWOptions

from pymimir.advanced.search import IAStarEagerEventHandler as AdvancedAStarEagerEventHandler
from pymimir.advanced.search import IBrFSEventHandler as AdvancedBrFSEventHandler
from pymimir.advanced.search import BrFSStatistics as AdvancedBrFSStatistics

from .wrapper_formalism import GroundAction, Problem, State


# -------------------
# Heuristic functions
# -------------------

class Heuristic:
    def compute_value(self, state: 'State', is_goal_state: bool) -> float:
        """
        Compute the heuristic value for a given state.

        :param state: The state for which to compute the heuristic.
        :type state: State
        :param is_goal_state: Whether the state is a goal state.
        :type is_goal_state: bool
        :return: The heuristic value.
        :rtype: float
        """
        raise NotImplementedError("This method should be overridden by subclasses.")

    def get_preferred_actions(self) -> 'set[GroundAction]':
        """
        Get preferred actions for the last computed state.

        :return: A set of GroundAction representing the preferred actions.
        :rtype: set[GroundAction]
        """
        raise NotImplementedError("This method should be overridden by subclasses.")


class AddHeuristic(Heuristic):
    def __init__(self, problem: 'Problem') -> None:
        super().__init__()
        assert isinstance(problem, Problem), "Problem must be an instance of Problem."
        self._problem = problem
        delete_relaxed = AdvancedDeleteRelaxedProblemExplorator(problem._advanced_problem)
        self._advanced_heuristic = AdvancedAddHeuristic.create(delete_relaxed)

    def get_problem(self) -> 'Problem':
        """
        Get the problem instance associated with this heuristic.

        :return: The problem instance.
        :rtype: Problem
        """
        return self._problem

    def compute_value(self, state: 'State', is_goal_state: bool) -> float:
        return self._advanced_heuristic.compute_heuristic(state._advanced_state, is_goal_state)

    def get_preferred_actions(self) -> 'set[GroundAction]':
        return { GroundAction(advanced_ground_action, self._problem) for advanced_ground_action in self._advanced_heuristic.get_preferred_actions().data }


class BlindHeuristic(Heuristic):
    def __init__(self, problem: 'Problem') -> None:
        super().__init__()
        assert isinstance(problem, Problem), "Problem must be an instance of Problem."
        self._problem = problem
        self._advanced_heuristic = AdvancedBlindHeuristic.create(problem._advanced_problem)

    def get_problem(self) -> 'Problem':
        """
        Get the problem instance associated with this heuristic.

        :return: The problem instance.
        :rtype: Problem
        """
        return self._problem

    def compute_value(self, state: 'State', is_goal_state: bool) -> float:
        return self._advanced_heuristic.compute_heuristic(state._advanced_state, is_goal_state)

    def get_preferred_actions(self) -> 'set[GroundAction]':
        return { GroundAction(advanced_ground_action, self._problem) for advanced_ground_action in self._advanced_heuristic.get_preferred_actions().data }


class MaxHeuristic(Heuristic):
    def __init__(self, problem: 'Problem') -> None:
        super().__init__()
        assert isinstance(problem, Problem), "Problem must be an instance of Problem."
        self._problem = problem
        delete_relaxed = AdvancedDeleteRelaxedProblemExplorator(problem._advanced_problem)
        self._advanced_heuristic = AdvancedMaxHeuristic.create(delete_relaxed)

    def get_problem(self) -> 'Problem':
        """
        Get the problem instance associated with this heuristic.

        :return: The problem instance.
        :rtype: Problem
        """
        return self._problem

    def compute_value(self, state: 'State', is_goal_state: bool) -> float:
        return self._advanced_heuristic.compute_heuristic(state._advanced_state, is_goal_state)

    def get_preferred_actions(self) -> 'set[GroundAction]':
        return { GroundAction(advanced_ground_action, self._problem) for advanced_ground_action in self._advanced_heuristic.get_preferred_actions().data }


class PerfectHeuristic(Heuristic):
    def __init__(self, problem: 'Problem') -> None:
        super().__init__()
        assert isinstance(problem, Problem), "Problem must be an instance of Problem."
        self._problem = problem
        self._advanced_heuristic = AdvancedPerfectHeuristic.create(problem._search_context)

    def get_problem(self) -> 'Problem':
        """
        Get the problem instance associated with this heuristic.

        :return: The problem instance.
        :rtype: Problem
        """
        return self._problem

    def compute_value(self, state: 'State', is_goal_state: bool) -> float:
        return self._advanced_heuristic.compute_heuristic(state._advanced_state, is_goal_state)

    def get_preferred_actions(self) -> 'set[GroundAction]':
        return { GroundAction(advanced_ground_action, self._problem) for advanced_ground_action in self._advanced_heuristic.get_preferred_actions().data }


class SetAddHeuristic(Heuristic):
    def __init__(self, problem: 'Problem') -> None:
        super().__init__()
        assert isinstance(problem, Problem), "Problem must be an instance of Problem."
        self._problem = problem
        delete_relaxed = AdvancedDeleteRelaxedProblemExplorator(problem._advanced_problem)
        self._advanced_heuristic = AdvancedSetAddHeuristic.create(delete_relaxed)

    def get_problem(self) -> 'Problem':
        """
        Get the problem instance associated with this heuristic.

        :return: The problem instance.
        :rtype: Problem
        """
        return self._problem

    def compute_value(self, state: 'State', is_goal_state: bool) -> float:
        return self._advanced_heuristic.compute_heuristic(state._advanced_state, is_goal_state)

    def get_preferred_actions(self) -> 'set[GroundAction]':
        return { GroundAction(advanced_ground_action, self._problem) for advanced_ground_action in self._advanced_heuristic.get_preferred_actions().data }


class FFHeuristic(Heuristic):
    def __init__(self, problem: 'Problem') -> None:
        super().__init__()
        assert isinstance(problem, Problem), "Problem must be an instance of Problem."
        self._problem = problem
        delete_relaxed = AdvancedDeleteRelaxedProblemExplorator(problem._advanced_problem)
        self._advanced_heuristic = AdvancedFFHeuristic.create(delete_relaxed)

    def get_problem(self) -> 'Problem':
        """
        Get the problem instance associated with this heuristic.

        :return: The problem instance.
        :rtype: Problem
        """
        return self._problem

    def compute_value(self, state: 'State', is_goal_state: bool) -> float:
        return self._advanced_heuristic.compute_heuristic(state._advanced_state, is_goal_state)

    def get_preferred_actions(self) -> 'set[GroundAction]':
        return { GroundAction(advanced_ground_action, self._problem) for advanced_ground_action in self._advanced_heuristic.get_preferred_actions().data }


# -----------------
# Search algorithms
# -----------------

class AdvancedHeuristicAdapter(AdvancedHeuristicBase):
    def __init__(self, heuristic: 'Heuristic', problem: 'Problem') -> None:
        """
        An adapter class to wrap a Heuristic instance to the internal interface.

        :param heuristic: The heuristic to wrap.
        :type heuristic: Heuristic
        :param problem: The problem associated with the heuristic.
        :type problem: Problem
        """
        super().__init__()
        assert isinstance(heuristic, Heuristic), "Heuristic must be an instance of Heuristic."
        assert isinstance(problem, Problem), "Problem must be an instance of Problem."
        self._heuristic = heuristic
        self._problem = problem

    def get_problem(self) -> 'Problem':
        return self._problem

    def compute_heuristic(self, state: 'AdvancedState', is_goal_state: bool) -> float:
        wrapped_state = State(state, self._problem)
        return self._heuristic.compute_value(wrapped_state, is_goal_state)

    def get_preferred_actions(self) -> AdvancedPreferredActions:
        preferred_actions = AdvancedPreferredActions()
        preferred_actions.data = { action._advanced_ground_action for action in self._heuristic.get_preferred_actions() }
        return preferred_actions


class SearchResult:
    def __init__(self, status, solution: 'Union[list[GroundAction], None]', solution_cost: 'Union[float, None]', goal_state: 'Union[State, None]') -> None:
        """
        A class to encapsulate the result of a search operation.

        :param status: The status of the search operation (e.g., 'solved', 'unsolvable').
        :type status: str
        :param solution: A list of GroundAction representing the solution path.
        :type solution: list[GroundAction] or None
        :param solution_cost: The total cost of the solution.
        :type solution_cost: float or None
        :param goal_state: The final state reached by the search.
        :type goal_state: State or None
        """
        self.status = status
        self.solution = solution
        self.solution_cost = solution_cost
        self.goal_state = goal_state


def astar_eager(problem: 'Problem',
                start_state: 'State',
                heuristic: 'Heuristic',
                max_time_seconds: float = -1,
                max_num_states: int = -1,
                on_expand_state: 'Union[Callable[[State], None], None]' = None,
                on_expand_goal_state: 'Union[Callable[[State], None], None]' = None,
                on_generate_state: 'Union[Callable[[State, GroundAction, float, State], None], None]' = None,
                on_prune_state: 'Union[Callable[[State], None], None]' = None,
                on_finish_f_layer: 'Union[Callable[[float], None], None]' = None) -> 'SearchResult':
    """
    A* search algorithm with eager evaluation.

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
    :param on_finish_f_layer: Callback function called when a layer of states is finished.
    :type on_finish_f_layer: Callable[[float], None]
    :return: A SearchResult object containing the status, solution, solution cost, and goal state.
    :rtype: SearchResult
    """
    assert isinstance(problem, Problem), "Problem must be an instance of Problem."
    assert isinstance(start_state, State), "Start state must be an instance of State."
    assert isinstance(heuristic, Heuristic), "Heuristic must be an instance of Heuristic."
    assert isinstance(max_time_seconds, (int, float)), "max_time_seconds must be an int or float."
    assert isinstance(max_num_states, int), "max_num_states must be an int."
    # Define the event handler with the provided callback functions.
    class EventHandler(AdvancedAStarEagerEventHandler):
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

        def on_finish_f_layer(self, arg):
            nonlocal problem, on_finish_f_layer
            if on_finish_f_layer:
                on_finish_f_layer(arg)

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

        # The following events are ignored in this interface.
        def on_close_state(self, arg0): pass
        def on_end_search(self, arg0, arg1, arg2, arg3, arg4, arg5): pass
        def on_exhausted(self): pass
        def on_generate_state_not_relaxed(self, arg0, arg1, arg2, arg3): pass
        def on_generate_state_relaxed(self, arg0, arg1, arg2, arg3): pass
        def on_solved(self, arg): pass
        def on_start_search(self, arg0, arg1, arg2): pass
        def on_unsolvable(self): pass

    # Create options for the A* search
    advanced_options = AdvancedAStarEagerOptions()
    if max_time_seconds > 0: advanced_options.max_time_in_ms = int(max_time_seconds * 1000)
    if max_num_states > 0: advanced_options.max_num_states = max_num_states
    advanced_options.start_state = start_state._advanced_state
    advanced_options.event_handler = EventHandler()
    # The A* search algorithm expects an advanced heuristic: get the internal one, or create a proxy
    if hasattr(heuristic, '_advanced_heuristic') and isinstance(heuristic._advanced_heuristic, AdvancedHeuristicBase):  # type: ignore
        advanced_heuristic = heuristic._advanced_heuristic  # type: ignore
    elif isinstance(heuristic, Heuristic):
        advanced_heuristic = AdvancedHeuristicAdapter(heuristic, problem)
    else:
        raise TypeError("Heuristic must be an instance of Heuristic.")
    # Invoke the A* search algorithm
    result = advanced_astar_eager(problem._search_context, advanced_heuristic, advanced_options)
    status = result.status.name.lower()
    solution = [GroundAction(x, problem) for x in result.plan.get_actions()] if result.plan else None
    solution_cost = result.plan.get_cost() if result.plan else None
    goal_state = State(result.goal_state, problem) if result.goal_state else None
    return SearchResult(status, solution, solution_cost, goal_state)


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
