""" This example illustrates the interface for the iterative width search algorithm and utilities.
"""

import pymimir.advanced.formalism as formalism
import pymimir.advanced.search as search

from pathlib import Path

ROOT_DIR = (Path(__file__).parent.parent.parent).absolute()


class CustomBlindHeuristic(search.IHeuristic):
    def __init__(self):
        super().__init__()

    def compute_heuristic(self, state : search.State, is_goal_state: bool) -> float:
        return 0.
    

class CustomGBFSEventHandler(search.IGBFSEventHandler):
    """A custom event handler to react on events during iw search.
    
    It is useful to collect custom statistics or create a search tree.
    """
    def __init__(self, problem: formalism.Problem):
        super().__init__()

        self.problem = problem
        self.statistics = search.GBFSStatistics()

    def on_expand_state(self, state : search.State):
        pass

    def on_expand_goal_state(self, state : search.State):
        pass

    def on_generate_state(self, state : search.State, action : formalism.GroundAction, action_cost: float, successor_state: search.State):
        pass

    def on_start_search(self, start_state : search.State):
        pass

    def on_end_search(self, num_reached_fluent_atoms : int, num_reached_derived_atoms: int, num_bytes_for_problem: int, num_bytes_for_nodes: int, num_states: int, num_nodes: int, num_actions: int, num_axioms: int):
        pass

    def on_solved(self, plan: search.Plan):
        pass

    def on_unsolvable(self):
        pass

    def on_exhausted(self):
        pass

    def get_statistics(self):
        return self.statistics


def main():
    domain_filepath = str(ROOT_DIR / "data" / "gripper" / "domain.pddl")
    problem_filepath = str(ROOT_DIR / "data" / "gripper" / "p-2-0.pddl")

    search_context = search.SearchContext.create(domain_filepath, problem_filepath)

    initial_state, _ = search_context.get_state_repository().get_or_create_initial_state()

    heuristic = CustomBlindHeuristic()
    event_handler = CustomGBFSEventHandler(search_context.get_problem())
    goal_strategy = search.ProblemGoalStrategy.create(search_context.get_problem())

    result = search.find_solution_gbfs(search_context, heuristic, initial_state, event_handler, goal_strategy)

    assert(result.status == search.SearchStatus.SOLVED)
    assert(len(result.plan) == 5)


if __name__ == "__main__":
    main()
