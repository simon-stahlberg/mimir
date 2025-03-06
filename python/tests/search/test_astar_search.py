import pymimir as mm

from pathlib import Path
from typing import List

ROOT_DIR = (Path(__file__).parent.parent.parent.parent).absolute()


class CustomBlindHeuristic(mm.IHeuristic):
    def __init__(self):
        super().__init__()

    def compute_heuristic(self, state : mm.State, is_goal_state: bool) -> float:
        return 0.


class CustomGoalCountHeuristic(mm.IHeuristic):
    def __init__(self, problem : mm.Problem):
        super().__init__()  
        self.problem = problem

        self.num_goal_literals = len(self.problem.get_fluent_goal_condition()) + len(self.problem.get_derived_goal_condition())

    def compute_heuristic(self, state : mm.State, is_goal_state: bool) -> float:
        num_satisfied_goal_literals = 0
        for literal in self.problem.get_fluent_goal_condition():
            if state.literal_holds(literal):
                num_satisfied_goal_literals += 1
        for literal in self.problem.get_derived_goal_condition():
            if state.literal_holds(literal):
                num_satisfied_goal_literals += 1

        return self.num_goal_literals - num_satisfied_goal_literals

class CustomAStarAlgorithmEventHandler2(mm.DefaultAStarAlgorithmEventHandler):
    pass

class CustomAStarAlgorithmEventHandler(mm.IAStarAlgorithmEventHandler):
    def __init__(self):
        super().__init__()

    def on_expand_state(self, state : mm.State, problem : mm.Problem):
        pass

    def on_expand_goal_state(self, state : mm.State, problem : mm.Problem):
        pass

    def on_generate_state(self, state : mm.State, action : mm.GroundAction, action_cost: float, successor_state: mm.State, problem : mm.Problem):
        pass

    def on_generate_state_relaxed(self, state : mm.State, action : mm.GroundAction, action_cost: float, successor_state: mm.State, problem : mm.Problem):
        pass

    def on_generate_state_not_relaxed(self, state : mm.State, action : mm.GroundAction, action_cost: float, successor_state: mm.State, problem : mm.Problem):
        pass

    def on_close_state(self, state : mm.State, problem : mm.Problem):
        pass

    def on_finish_f_layer(self, f_value: float):
        pass

    def on_prune_state(self, state : mm.State, problem : mm.Problem):
        pass

    def on_start_search(self, start_state : mm.State, problem : mm.Problem):
        pass

    def on_end_search(self, num_reached_fluent_atoms : int, num_reached_derived_atoms: int, num_bytes_for_unextended_state_portion: int, num_bytes_for_extended_state_portion: int, num_bytes_for_nodes: int, num_bytes_for_actions: int, num_bytes_for_axioms: int, num_states: int, num_nodes: int, num_actions: int, num_axioms: int):
        pass

    def on_solved(self, plan: mm.Plan, problem: mm.Problem):
        pass

    def on_unsolvable(self):
        pass

    def on_exhausted(self):
        pass

def test_astar_search():
    """ Test parsing a PDDL domain and problem file.
    """
    domain_filepath = str(ROOT_DIR / "data" / "gripper" / "domain.pddl")
    problem_filepath = str(ROOT_DIR / "data" / "gripper" / "test_problem.pddl")
    search_context = mm.SearchContext(domain_filepath, problem_filepath)

    blind_heuristic = CustomBlindHeuristic()
    goal_count_heuristic = CustomGoalCountHeuristic(search_context.get_problem())
    event_handler = CustomAStarAlgorithmEventHandler()
    initial_state = search_context.get_state_repository().get_or_create_initial_state()

    result = mm.find_solution_astar(search_context, blind_heuristic, initial_state, mm.DefaultAStarAlgorithmEventHandler(False))

    assert result.status == mm.SearchStatus.SOLVED
    assert len(result.plan) == 3
