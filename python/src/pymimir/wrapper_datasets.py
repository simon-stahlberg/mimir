from typing import Iterable, Union

from pymimir.advanced.datasets import StateSpace as AdvancedStateSpace
from pymimir.advanced.datasets import StateSpaceOptions as AdvancedStateSpaceOptions
from pymimir.advanced.datasets import StateSpaceSampler as AdvancedStateSpaceSampler

from .wrapper_formalism import GroundAction, Problem, State


# --------
# Datasets
# --------

class StateLabel:
    def __init__(
        self,
        steps_to_goal: int,
        cost_to_goal: float,
        is_initial: bool,
        is_goal: bool,
        is_dead_end: bool
    ):
        """
        A class representing a labeled state with additional properties.

        :param steps_to_goal: The number of steps to reach the goal from this state.
        :type steps_to_goal: int
        :param cost_to_goal: The lowest cost to reach the goal from this state.
        :type cost_to_goal: float
        :param is_initial: Whether this state is the initial state.
        :type is_initial: bool
        :param is_goal: Whether this state is a goal state.
        :type is_goal: bool
        :param is_dead_end: Whether this state is a dead-end state.
        :type is_dead_end: bool
        """
        self.steps_to_goal = steps_to_goal
        self.cost_to_goal = cost_to_goal
        self.is_initial = is_initial
        self.is_goal = is_goal
        self.is_dead_end = is_dead_end


class StateSpaceSampler:
    """
    A class representing a state space, which is a collection of states.
    """

    def __init__(self, advanced_state_space_sampler: 'AdvancedStateSpaceSampler', problem: 'Problem'):
        """
        Internal constructor for the StateSpaceSampler class; to create a state space sampler, use the new() method.

        :param advanced_state_space_sampler: An instance of AdvancedStateSpaceSampler.
        :type advanced_state_space_sampler: AdvancedStateSpaceSampler
        :param problem: An instance of Problem that this state space is associated with.
        :type problem: Problem
        """
        assert isinstance(advanced_state_space_sampler, AdvancedStateSpaceSampler), "Invalid state space sampler type."
        assert isinstance(problem, Problem), "Invalid problem type."
        self._advanced_state_space_sampler = advanced_state_space_sampler
        self._problem = problem

    @staticmethod
    def new(problem: 'Problem', max_states: Union[int, None] = None, symmetry_pruning: Union[bool, None] = None) -> 'Union[StateSpaceSampler, None]':
        """
        Create a new state space from a given problem.

        :param problem: The problem to create the state space for.
        :type problem: Problem
        :param max_states: The maximum number of states before aborting, or None for no limit.
        :type max_states: int or None
        :param symmetry_pruning: Whether to apply symmetry pruning to the state space, or None for default behavior.
        :type symmetry_pruning: bool or None
        :return: A new StateSpaceSampler instance.
        :rtype: Union[StateSpaceSampler, None]
        """
        if max_states is not None and max_states <= 0:
            raise ValueError("max_states must be a positive integer or None.")
        if symmetry_pruning is not None and not isinstance(symmetry_pruning, bool):
            raise ValueError("symmetry_pruning must be a boolean or None.")
        # Set up the options.
        state_space_options = AdvancedStateSpaceOptions()
        if max_states is not None:
            state_space_options.max_num_states = max_states
        if symmetry_pruning is not None:
            state_space_options.symmetry_pruning = symmetry_pruning
        # Create the advanced state space.
        result = AdvancedStateSpace.create(problem._search_context, state_space_options)
        if result is None:
            return None
        advanced_state_space, _ = result
        advanced_state_space_sampler = AdvancedStateSpaceSampler(advanced_state_space)
        return StateSpaceSampler(advanced_state_space_sampler, problem)

    def get_problem(self) -> 'Problem':
        """
        Get the problem instance associated with this state space sampler.

        :return: The problem instance.
        :rtype: Problem
        """
        return self._problem

    def set_seed(self, seed: int) -> None:
        """
        Set the random seed for the state space sampler.
        This allows for reproducibility in sampling states.

        :param seed: The seed to set for the random number generator.
        :type seed: int
        """
        if not isinstance(seed, int):
            raise ValueError("Seed must be an integer.")
        self._advanced_state_space_sampler.set_seed(seed)

    def get_state(self, index: int) -> 'State':
        """
        Get the state with the given index in the state space.

        :param index: The index of the state.
        :type index: int
        :return A State object with the given index.
        :rtype State
        """
        assert isinstance(index, int), "Index must be an integer."
        assert index >= 0, "Index must not be negative."
        advanced_state = self._advanced_state_space_sampler.get_state(index)
        return State(advanced_state, self.get_problem())

    def get_states(self) -> 'list[State]':
        """
        Get the states in the state space.

        :return A list of State objects.
        :rtype list[State]
        """
        advanced_states = self._advanced_state_space_sampler.get_states()
        return [State(advanced_state, self.get_problem()) for advanced_state in advanced_states]

    def get_state_label(self, state: 'State') -> 'StateLabel':
        """
        Get the label for a given state.
        Returns a StateLabel object with properties of the given state.

        :param state: The state for which to get the label.
        :type state: State
        :return: A StateLabel object containing the properties of the state.
        :rtype: StateLabel
        """
        if not isinstance(state, State):
            raise ValueError("state must be an instance of State.")
        advanced_state = state._advanced_state
        steps_to_goal = self._advanced_state_space_sampler.get_steps_to_goal(advanced_state)
        cost_to_goal = self._advanced_state_space_sampler.get_cost_to_goal(advanced_state)
        is_initial = self._advanced_state_space_sampler.is_initial_state(advanced_state)
        is_goal = self._advanced_state_space_sampler.is_goal_state(advanced_state)
        is_dead_end = self._advanced_state_space_sampler.is_dead_end_state(advanced_state)
        return StateLabel(steps_to_goal, cost_to_goal, is_initial, is_goal, is_dead_end)

    def num_states(self) -> int:
        """
        Get the number of states in the state space.
        Returns the total number of states.

        :return: The number of states in the state space.
        :rtype: int
        """
        return self._advanced_state_space_sampler.get_num_states()

    def num_dead_end_states(self) -> int:
        """
        Get the number of dead-end states in the state space.
        Returns the total number of dead-end states.

        :return: The number of dead-end states in the state space.
        :rtype: int
        """
        return self._advanced_state_space_sampler.get_num_dead_end_states()

    def num_alive_states(self) -> int:
        """
        Get the number of alive states in the state space.
        Returns the total number of alive states.

        :return: The number of alive states in the state space.
        :rtype: int
        """
        return self._advanced_state_space_sampler.get_num_alive_states()

    def max_steps_to_goal(self) -> int:
        """
        Get the maximum number of steps to reach the goal from any state in the state space.
        Dead-end states are not considered in this calculation.
        Returns the maximum steps to goal.

        :return: The maximum number of steps to reach the goal.
        :rtype: int
        """
        return self._advanced_state_space_sampler.get_max_steps_to_goal()

    def sample_state(self) -> 'State':
        """
        Sample a state from the state space.
        Returns a State object with the sampled state and its properties.

        :return: A State object representing the sampled state.
        :rtype: State
        """
        advanced_state = self._advanced_state_space_sampler.sample_state()
        return State(advanced_state, self._problem)

    def sample_states(self, num_states: int) -> Iterable['State']:
        """
        Sample a specified number of states from the state space.
        Returns an iterable of State objects.

        :param num_states: The number of states to sample.
        :type num_states: int
        :return: An iterable of State objects.
        :rtype: Iterable[State]
        """
        if num_states <= 0:
            raise ValueError("num_states must be a positive integer.")
        return (self.sample_state() for _ in range(num_states))

    def sample_dead_end_state(self) -> 'State':
        """
        Sample a dead-end state from the state space.
        Returns a State object with the sampled dead-end state.

        :return: A State object representing the sampled dead-end state.
        :rtype: State
        """
        advanced_state = self._advanced_state_space_sampler.sample_dead_end_state()
        return State(advanced_state, self._problem)

    def sample_dead_end_states(self, num_states: int) -> Iterable['State']:
        """
        Sample a specified number of dead-end states from the state space.
        Returns an iterable of State objects.

        :param num_states: The number of dead-end states to sample.
        :type num_states: int
        :return: An iterable of State objects representing dead-end states.
        :rtype: Iterable[State]
        """
        if num_states <= 0:
            raise ValueError("num_states must be a positive integer.")
        return (self.sample_dead_end_state() for _ in range(num_states))

    def sample_state_n_steps_from_goal(self, n: int) -> 'State':
        """
        Sample a state that is n steps away from the goal.
        Returns a State object with the sampled state.

        :param n: The number of steps away from the goal.
        :type n: int
        :return: A State object representing the sampled state n steps from the goal.
        :rtype: State
        """
        if n < 0:
            raise ValueError("n must be a non-negative integer.")
        advanced_state = self._advanced_state_space_sampler.sample_state_n_steps_from_goal(n)
        return State(advanced_state, self._problem)

    def sample_states_n_steps_from_goal(self, n: int, num_states: int) -> Iterable['State']:
        """
        Sample a specified number of states that are n steps away from the goal.
        Returns an iterable of State objects.

        :param n: The number of steps away from the goal.
        :type n: int
        :param num_states: The number of states to sample.
        :type num_states: int
        :return: An iterable of State objects representing states n steps from the goal.
        :rtype: Iterable[State]
        """
        if n < 0:
            raise ValueError("n must be a non-negative integer.")
        if num_states <= 0:
            raise ValueError("num_states must be a positive integer.")
        return (self.sample_state_n_steps_from_goal(n) for _ in range(num_states))

    def get_forward_transitions(self, state: 'State') -> Iterable['tuple[GroundAction, State]']:
        """
        Get the forward transitions from a given state.
        Returns an iterable of tuples containing the ground action and the resulting state.

        :param state: The state from which to get the forward transitions.
        :type state: State
        :return: An iterable of tuples (GroundAction, State) representing the forward transitions.
        :rtype: Iterable[tuple[GroundAction, State]]
        """
        if not isinstance(state, State):
            raise ValueError("state must be an instance of State.")
        advanced_state = state._advanced_state
        advanced_transitions = self._advanced_state_space_sampler.get_forward_transitions(advanced_state)
        return (
            (GroundAction(advanced_action, self._problem), State(advanced_successor_state, self._problem))
            for advanced_action, advanced_successor_state in advanced_transitions
        )

    def get_backward_transitions(self, state: 'State') -> Iterable['tuple[GroundAction, State]']:
        """
        Get the backward transitions to a given state.
        Returns an iterable of tuples containing the ground action and the predecessor state.

        :param state: The state to which to get the backward transitions.
        :type state: State
        :return: An iterable of tuples (GroundAction, State) representing the backward transitions.
        :rtype: Iterable[tuple[GroundAction, State]]
        """
        if not isinstance(state, State):
            raise ValueError("state must be an instance of State.")
        advanced_state = state._advanced_state
        advanced_transitions = self._advanced_state_space_sampler.get_backward_transitions(advanced_state)
        return (
            (GroundAction(advanced_action, self._problem), State(advanced_predecessor_state, self._problem))
            for advanced_action, advanced_predecessor_state in advanced_transitions
        )

    def __str__(self) -> str:
        """
        Return a string representation of the StateSpaceSampler.

        :return: A string representation of the StateSpaceSampler.
        :rtype: str
        """
        num_states = self._advanced_state_space_sampler.get_num_states()
        return f"StateSpaceSampler({num_states} states)"

    def __repr__(self) -> str:
        """
        Return a string representation of the StateSpaceSampler.

        :return: A string representation of the StateSpaceSampler.
        :rtype: str
        """
        return self.__str__()
