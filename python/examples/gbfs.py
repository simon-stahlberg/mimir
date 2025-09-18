""" This example illustrates the interface for the greedy best first search algorithm.
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
    

class CustomGBFSEventHandler(search.IGBFSEagerEventHandler):
    """A custom event handler to react on events during iw search.
    
    It is useful to collect custom statistics or create a search tree.
    """
    def __init__(self, problem: formalism.Problem):
        super().__init__()

        self.problem = problem
        self.statistics = search.GBFSEagerStatistics()

    def on_expand_state(self, state : search.State):
        pass

    def on_expand_goal_state(self, state : search.State):
        pass

    def on_generate_state(self, state : search.State, action : formalism.GroundAction, action_cost: float, successor_state: search.State):
        pass

    def on_start_search(self, start_state : search.State, g_value : float, h_value : float):
        pass

    def on_new_best_h_value(self, h_value : float):
        pass

    def on_end_search(self, num_reached_fluent_atoms : int, num_reached_derived_atoms: int, num_states: int, num_nodes: int, num_actions: int, num_axioms: int):
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

    # Create the search context
    problem = formalism.Problem.create(domain_filepath, problem_filepath, formalism.ParserOptions())
    applicable_action_generator = search.KPKCLiftedApplicableActionGenerator.create(problem)
    axiom_evaluator = search.KPKCLiftedAxiomEvaluator.create(problem)
    state_repository = search.StateRepository.create(axiom_evaluator)
    search_context = search.SearchContext.create(problem, applicable_action_generator, state_repository)

    # Create some heuristics
    delete_relaxation = search.DeleteRelaxedProblemExplorator(problem)
    heuristic = search.FFHeuristic.create(delete_relaxation)

    # Create the goal strategy
    goal_strategy = search.ProblemGoalStrategy.create(search_context.get_problem())

    # GBFS Eager: evaluates heuristic on generated states; ignores preferred actions
    gbfs_eager_options = search.GBFSEagerOptions()
    gbfs_eager_options.event_handler =  search.DefaultGBFSEagerEventHandler(search_context.get_problem())
    gbfs_eager_options.goal_strategy = goal_strategy

    result = search.find_solution_gbfs_eager(search_context, heuristic, gbfs_eager_options)

    assert(result.status == search.SearchStatus.SOLVED)
    assert(len(result.plan) == 5)

    print(gbfs_eager_options.event_handler.get_statistics())
    print(result.plan)

    # GBFS Lazy: evaluates heuristic on expanded states; uses preferred actions
    gbfs_lazy_options = search.GBFSLazyOptions()
    gbfs_lazy_options.event_handler =  search.DefaultGBFSLazyEventHandler(search_context.get_problem())
    gbfs_lazy_options.goal_strategy = goal_strategy

    result = search.find_solution_gbfs_lazy(search_context, heuristic, gbfs_lazy_options)

    assert(result.status == search.SearchStatus.SOLVED)
    assert(len(result.plan) == 5)

    print(gbfs_lazy_options.event_handler.get_statistics())
    print(result.plan)


if __name__ == "__main__":
    main()
