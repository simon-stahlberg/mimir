from formalism import *

from pymimir.advanced.search import State as AdvancedState
from pymimir.advanced.search import LiftedApplicableActionGenerator as AdvancedLiftedApplicableActionGenerator

# General comment: Avoid caching as the objects in this class tend to be very many.

class State:
    """
    State class representing a node in the search space.
    """
    def __init__(self, problem: Problem, advanced_state: AdvancedState):
        self._problem = problem
        self._advanced_state = advanced_state

    def get_index(self) -> int:
        """
        Returns the index of the state.
        """
        return self._advanced_state.get_index()

    def get_ground_atoms(self, skip_static = True, skip_fluent = False, skip_derived = False) -> list[GroundAtom]:
        """
        Returns the ground atoms of the state.
        """
        ground_atoms = []
        if not skip_static:
             static_ground_atoms = self._problem.get_initial_atoms(skip_static = False, skip_fluent = True, skip_derived = True)
             ground_atoms.extend([GroundAtom(x) for x in static_ground_atoms])
        if not skip_fluent:
            fluent_indices = self._advanced_state.get_fluent_atoms()
            fluent_ground_atoms = self._problem._advanced_problem.get_repositories().get_fluent_ground_atoms_from_indices(fluent_indices)
            ground_atoms.extend([GroundAtom(x) for x in fluent_ground_atoms])
        if not skip_derived:
            derived_indices = self._advanced_state.get_derived_atoms()
            derived_ground_atoms = self._problem._advanced_problem.get_repositories().get_derived_ground_atoms_from_indices(derived_indices)
            ground_atoms.extend([GroundAtom(x) for x in derived_ground_atoms])
        return ground_atoms

    def literal_holds(self, literal: GroundLiteral) -> bool:
        """
        Checks if a literal holds in the state.
        """
        return self._advanced_state.literal_holds(literal)

    def literals_hold(self, atoms: list[GroundLiteral]) -> bool:
        """
        Checks if all literals hold in the state.
        """
        return self._advanced_state.literals_hold(atoms)

    def __str__(self) -> str:
        """
        Returns a string representation of the state.
        """
        return self._advanced_state.to_string(self._problem._advanced_problem)

    def __repr__(self) -> str:
        """
        Returns a string representation of the state.
        """
        return self.__str__()

    def __hash__(self) -> int:
        """
        Returns a hash of the state.
        """
        return hash(self._advanced_state)

    def __eq__(self, other) -> bool:
        """
        Checks if two states are equal.
        """
        if not isinstance(other, State):
            return False
        return self._advanced_state == other._advanced_state


class ApplicableActionGenerator:
    """
    Class to generate applicable actions for a given state.
    """
    def __init__(self, problem: Problem):
        self._problem = problem

    def get_applicable_actions(self, state: State) -> list[Action]:
        """
        Returns the applicable actions for the given state.
        """
        return self._problem.get_applicable_actions(state._advanced_state)