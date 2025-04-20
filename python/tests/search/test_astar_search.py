import pymimir.advanced.formalism as formalism
import pymimir.advanced.search as search

from pathlib import Path

ROOT_DIR = (Path(__file__).parent.parent.parent.parent).absolute()


class CustomBlindHeuristic(search.IHeuristic):
    def __init__(self):
        super().__init__()

    def compute_heuristic(self, state : search.State, is_goal_state: bool) -> float:
        return 0.


class CustomEventHandler(search.IAStarEventHandler):
    def __init__(self):
        super().__init__()

        self.statistics = search.AStarStatistics()

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

def test_astar_search():
    """ Test parsing a PDDL domain and problem file.
    """
    domain_filepath = str(ROOT_DIR / "data" / "gripper" / "domain.pddl")
    problem_filepath = str(ROOT_DIR / "data" / "gripper" / "test_problem.pddl")
    search_context = search.SearchContext.create(domain_filepath, problem_filepath)

    blind_heuristic = CustomBlindHeuristic()
    initial_state, _ = search_context.get_state_repository().get_or_create_initial_state()
    event_handler = CustomEventHandler()
    problem_goal_stategy = search.ProblemGoalStrategy(search_context.get_problem())
    no_pruning_strategy = search.NoPruningStrategy()

    result = search.find_solution_astar(search_context, blind_heuristic, initial_state, event_handler, problem_goal_stategy, no_pruning_strategy)

    assert result.status == search.SearchStatus.SOLVED
    assert len(result.plan) == 3
