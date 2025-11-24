from typing import Union

from .wrapper_formalism import GroundAction, State


# -----------------------
# Common search interface
# -----------------------

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
