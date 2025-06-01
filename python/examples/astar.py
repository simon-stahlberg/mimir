""" This example illustrates the interface for the astar search algorithm.
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


class CustomEventHandler(search.IAStarEagerEventHandler):
    def __init__(self):
        super().__init__()

        self.statistics = search.AStarEagerStatistics()

    def on_expand_state(self, state : search.State):
        pass

    def on_expand_goal_state(self, state : search.State):
        pass

    def on_generate_state(self, state : search.State, action : formalism.GroundAction, action_cost: float, successor_state: search.State):
        pass

    def on_generate_state_relaxed(self, state : search.State, action : formalism.GroundAction, action_cost: float, successor_state: search.State):
        pass

    def on_generate_state_not_relaxed(self, state : search.State, action : formalism.GroundAction, action_cost: float, successor_state: search.State):
        pass

    def on_close_state(self, state : search.State):
        pass

    def on_finish_f_layer(self, f_value: float):
        pass

    def on_prune_state(self, state : search.State):
        pass

    def on_start_search(self, start_state : search.State, g_value : float, h_value : float):
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

    # Create the search context
    problem = formalism.Problem.create(domain_filepath, problem_filepath, formalism.ParserOptions())
    applicable_action_generator = search.LiftedApplicableActionGenerator(problem)
    axiom_evaluator = search.LiftedAxiomEvaluator(problem)
    state_repository = search.StateRepository(axiom_evaluator)
    search_context = search.SearchContext.create(problem, applicable_action_generator, state_repository)

    # Create some heuristics
    delete_relaxation = search.DeleteRelaxedProblemExplorator(problem)
    heuristic = search.MaxHeuristic(delete_relaxation)

    # Create the goal strategy
    goal_strategy = search.ProblemGoalStrategy.create(search_context.get_problem())

    # GBFS Eager: evaluates heuristic on generated states; ignores preferred actions
    astar_eager_options = search.AStarEagerOptions()
    astar_eager_options.event_handler = search.DefaultAStarEagerEventHandler(search_context.get_problem())
    astar_eager_options.goal_strategy = goal_strategy

    result = search.find_solution_astar_eager(search_context, heuristic, astar_eager_options)

    assert(result.status == search.SearchStatus.SOLVED)
    assert(len(result.plan) == 5)

    print(astar_eager_options.event_handler.get_statistics())
    print(result.plan)

    # GBFS Lazy: evaluates heuristic on expanded states; uses preferred actions
    astar_lazy_options = search.AStarLazyOptions()
    astar_lazy_options.event_handler = search.DefaultAStarLazyEventHandler(search_context.get_problem())
    astar_lazy_options.goal_strategy = goal_strategy

    result = search.find_solution_astar_lazy(search_context, heuristic, astar_lazy_options)

    assert(result.status == search.SearchStatus.SOLVED)
    assert(len(result.plan) == 5)

    print(astar_lazy_options.event_handler.get_statistics())
    print(result.plan)


if __name__ == "__main__":
    main()
