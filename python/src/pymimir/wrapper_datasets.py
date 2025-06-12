from typing import Iterable

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
        is_initial: bool,
        is_goal: bool,
        is_dead_end: bool
    ):
        """A class representing a labeled state with additional properties."""
        self.steps_to_goal = steps_to_goal
        self.is_initial = is_initial
        self.is_goal = is_goal
        self.is_dead_end = is_dead_end


class StateSpaceSampler:
    """
    A class representing a state space, which is a collection of states.
    """

    def __init__(self, advanced_state_space_sampler: 'AdvancedStateSpaceSampler', problem: 'Problem'):
        assert isinstance(advanced_state_space_sampler, AdvancedStateSpaceSampler), "Invalid state space sampler type."
        assert isinstance(problem, Problem), "Invalid problem type."
        self._advanced_state_space_sampler = advanced_state_space_sampler
        self._problem = problem

    @staticmethod
    def new(problem: 'Problem', max_states: int = None, symmetry_pruning: bool = None) -> 'StateSpaceSampler':
        """
        Create a new state space from a given problem.
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
        advanced_state_space, _ = AdvancedStateSpace.create(problem._search_context, state_space_options)
        if advanced_state_space is None:
            return None
        advanced_state_space_sampler = AdvancedStateSpaceSampler(advanced_state_space)
        return StateSpaceSampler(advanced_state_space_sampler, problem)

    def set_seed(self, seed: int) -> None:
        """
        Set the random seed for the state space sampler.
        This allows for reproducibility in sampling states.
        """
        if not isinstance(seed, int):
            raise ValueError("Seed must be an integer.")
        self._advanced_state_space_sampler.set_seed(seed)

    def get_state_label(self, state: 'State') -> 'StateLabel':
        """
        Get the label for a given state.
        Returns a StateLabel object with properties of the given state.
        """
        if not isinstance(state, State):
            raise ValueError("state must be an instance of State.")
        advanced_state = state._advanced_state
        steps_to_goal = self._advanced_state_space_sampler.get_steps_to_goal(advanced_state)
        is_initial = self._advanced_state_space_sampler.is_initial_state(advanced_state)
        is_goal = self._advanced_state_space_sampler.is_goal_state(advanced_state)
        is_dead_end = self._advanced_state_space_sampler.is_dead_end_state(advanced_state)
        return StateLabel(steps_to_goal, is_initial, is_goal, is_dead_end)

    def sample_state(self) -> 'State':
        """
        Sample a state from the state space.
        Returns a State object with the sampled state and its properties.
        """
        advanced_state = self._advanced_state_space_sampler.sample_state()
        return State(advanced_state, self._problem)

    def sample_states(self, num_states: int) -> Iterable['State']:
        """
        Sample a specified number of states from the state space.
        Returns an iterable of State objects.
        """
        if num_states <= 0:
            raise ValueError("num_states must be a positive integer.")
        return (self.sample_state() for _ in range(num_states))

    def sample_dead_end_state(self) -> 'State':
        """
        Sample a dead-end state from the state space.
        Returns a State object with the sampled dead-end state.
        """
        advanced_state = self._advanced_state_space_sampler.sample_dead_end_state()
        return State(advanced_state, self._problem)

    def sample_dead_end_states(self, num_states: int) -> Iterable['State']:
        """
        Sample a specified number of dead-end states from the state space.
        Returns an iterable of State objects.
        """
        if num_states <= 0:
            raise ValueError("num_states must be a positive integer.")
        return (self.sample_dead_end_state() for _ in range(num_states))

    def sample_state_n_steps_from_goal(self, n: int) -> 'State':
        """
        Sample a state that is n steps away from the goal.
        Returns a State object with the sampled state.
        """
        if n < 0:
            raise ValueError("n must be a non-negative integer.")
        advanced_state = self._advanced_state_space_sampler.sample_state_n_steps_from_goal(n)
        return State(advanced_state, self._problem)

    def sample_states_n_steps_from_goal(self, n: int, num_states: int) -> Iterable['State']:
        """
        Sample a specified number of states that are n steps away from the goal.
        Returns an iterable of State objects.
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
        num_states = self._advanced_state_space_sampler.get_num_states()
        return f"StateSpaceSampler({num_states} states)"

    def __repr__(self) -> str:
        return self.__str__()
