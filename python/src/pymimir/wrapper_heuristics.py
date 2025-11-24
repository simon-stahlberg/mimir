from abc import ABC, abstractmethod
from typing import Union

from pymimir.advanced.formalism import GroundConjunctiveCondition as AdvancedGroundConjunctiveCondition
from pymimir.advanced.search import AddHeuristic as AdvancedAddHeuristic
from pymimir.advanced.search import BlindHeuristic as AdvancedBlindHeuristic
from pymimir.advanced.search import LiftedGrounder as AdvancedLiftedGrounder
from pymimir.advanced.search import FFHeuristic as AdvancedFFHeuristic
from pymimir.advanced.search import IHeuristic as AdvancedHeuristicBase
from pymimir.advanced.search import MaxHeuristic as AdvancedMaxHeuristic
from pymimir.advanced.search import PerfectHeuristic as AdvancedPerfectHeuristic
from pymimir.advanced.search import PreferredActions as AdvancedPreferredActions
from pymimir.advanced.search import SetAddHeuristic as AdvancedSetAddHeuristic
from pymimir.advanced.search import State as AdvancedState

from .wrapper_formalism import GroundAction, Problem, State, GroundConjunctiveCondition


# -------------------
# Heuristic functions
# -------------------

class Heuristic(ABC):
    """
    Abstract base class for heuristic functions used in search algorithms.
    """

    @abstractmethod
    def compute_value(self, state: 'State', goal: 'Union[GroundConjunctiveCondition, None]' = None) -> float:
        """
        Compute the heuristic value for a given state.

        :param state: The state for which to compute the heuristic.
        :type state: State
        :param goal: The goal condition to compute the heuristic against (optional).
        :type goal: Union[GroundConjunctiveCondition, None]
        :return: The heuristic value.
        :rtype: float
        """
        raise NotImplementedError("This method should be overridden by subclasses.")

    @abstractmethod
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
        delete_relaxed = AdvancedLiftedGrounder(problem._advanced_problem)
        self._advanced_heuristic = AdvancedAddHeuristic.create(delete_relaxed)

    def get_problem(self) -> 'Problem':
        """
        Get the problem instance associated with this heuristic.

        :return: The problem instance.
        :rtype: Problem
        """
        return self._problem

    def compute_value(self, state: 'State', goal: 'Union[GroundConjunctiveCondition, None]' = None) -> float:
        return self._advanced_heuristic.compute_heuristic(state._advanced_state, goal._advanced_condition if goal else None)

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

    def compute_value(self, state: 'State', goal: 'Union[GroundConjunctiveCondition, None]' = None) -> float:
        return self._advanced_heuristic.compute_heuristic(state._advanced_state, goal._advanced_condition if goal else None)

    def get_preferred_actions(self) -> 'set[GroundAction]':
        return { GroundAction(advanced_ground_action, self._problem) for advanced_ground_action in self._advanced_heuristic.get_preferred_actions().data }


class MaxHeuristic(Heuristic):
    def __init__(self, problem: 'Problem') -> None:
        super().__init__()
        assert isinstance(problem, Problem), "Problem must be an instance of Problem."
        self._problem = problem
        delete_relaxed = AdvancedLiftedGrounder(problem._advanced_problem)
        self._advanced_heuristic = AdvancedMaxHeuristic.create(delete_relaxed)

    def get_problem(self) -> 'Problem':
        """
        Get the problem instance associated with this heuristic.

        :return: The problem instance.
        :rtype: Problem
        """
        return self._problem

    def compute_value(self, state: 'State', goal: 'Union[GroundConjunctiveCondition, None]' = None) -> float:
        return self._advanced_heuristic.compute_heuristic(state._advanced_state, goal._advanced_condition if goal else None)

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

    def compute_value(self, state: 'State', goal: 'Union[GroundConjunctiveCondition, None]' = None) -> float:
        return self._advanced_heuristic.compute_heuristic(state._advanced_state, goal._advanced_condition if goal else None)

    def get_preferred_actions(self) -> 'set[GroundAction]':
        return { GroundAction(advanced_ground_action, self._problem) for advanced_ground_action in self._advanced_heuristic.get_preferred_actions().data }


class SetAddHeuristic(Heuristic):
    def __init__(self, problem: 'Problem') -> None:
        super().__init__()
        assert isinstance(problem, Problem), "Problem must be an instance of Problem."
        self._problem = problem
        delete_relaxed = AdvancedLiftedGrounder(problem._advanced_problem)
        self._advanced_heuristic = AdvancedSetAddHeuristic.create(delete_relaxed)

    def get_problem(self) -> 'Problem':
        """
        Get the problem instance associated with this heuristic.

        :return: The problem instance.
        :rtype: Problem
        """
        return self._problem

    def compute_value(self, state: 'State', goal: 'Union[GroundConjunctiveCondition, None]' = None) -> float:
        return self._advanced_heuristic.compute_heuristic(state._advanced_state, goal._advanced_condition if goal else None)

    def get_preferred_actions(self) -> 'set[GroundAction]':
        return { GroundAction(advanced_ground_action, self._problem) for advanced_ground_action in self._advanced_heuristic.get_preferred_actions().data }


class FFHeuristic(Heuristic):
    def __init__(self, problem: 'Problem') -> None:
        super().__init__()
        assert isinstance(problem, Problem), "Problem must be an instance of Problem."
        self._problem = problem
        delete_relaxed = AdvancedLiftedGrounder(problem._advanced_problem)
        self._advanced_heuristic = AdvancedFFHeuristic.create(delete_relaxed)

    def get_problem(self) -> 'Problem':
        """
        Get the problem instance associated with this heuristic.

        :return: The problem instance.
        :rtype: Problem
        """
        return self._problem

    def compute_value(self, state: 'State', goal: 'Union[GroundConjunctiveCondition, None]' = None) -> float:
        return self._advanced_heuristic.compute_heuristic(state._advanced_state, goal._advanced_condition if goal else None)

    def get_preferred_actions(self) -> 'set[GroundAction]':
        return { GroundAction(advanced_ground_action, self._problem) for advanced_ground_action in self._advanced_heuristic.get_preferred_actions().data }


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

    def compute_heuristic(self, state: 'AdvancedState', goal: 'Union[AdvancedGroundConjunctiveCondition, None]' = None) -> float:
        wrapped_state = State(state, self._problem)
        wrapped_goal = GroundConjunctiveCondition(goal, self._problem) if goal else None
        return self._heuristic.compute_value(wrapped_state, wrapped_goal)

    def get_preferred_actions(self) -> AdvancedPreferredActions:
        preferred_actions = AdvancedPreferredActions()
        preferred_actions.data = { action._advanced_ground_action for action in self._heuristic.get_preferred_actions() }
        return preferred_actions
