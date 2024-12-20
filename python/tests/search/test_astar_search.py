import pymimir as mm

from pathlib import Path
from typing import List

ROOT_DIR = (Path(__file__).parent.parent.parent.parent).absolute()


class CustomBlindHeuristic(mm.IHeuristic):
    def __init__(self):
        mm.IHeuristic.__init__(self)  # Without this, a TypeError is raised.

    def compute_heuristic(self, state : mm.State) -> float:
        return 0.


class CustomGoalCountHeuristic(mm.IHeuristic):
    def __init__(self, problem : mm.Problem, pddl_repositories: mm.PDDLRepositories):
        mm.IHeuristic.__init__(self)  # Without this, a TypeError is raised.
        self.problem = problem
        self.pddl_repositories = pddl_repositories

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


class CustomAStarAlgorithmEventHandler(mm.AStarAlgorithmEventHandlerBase):
    def __init__(self, quiet = True):
        """
        AStarAlgorithmEventHandlerBase collects statistics and
        calls callback implementations only if quiet = False.
        """
        mm.AStarAlgorithmEventHandlerBase.__init__(self, quiet)  # Without this, a TypeError is raised.

    def on_expand_state_impl(self, state : mm.State, problem : mm.Problem, pddl_repositories : mm.PDDLRepositories):
        pass

    def on_generate_state_impl(self, state : mm.State, action : mm.GroundAction, action_cost: float, problem : mm.Problem, pddl_repositories : mm.PDDLRepositories):
        pass

    def on_generate_state_relaxed_impl(self, state : mm.State, action : mm.GroundAction, action_cost: float, problem : mm.Problem, pddl_repositories : mm.PDDLRepositories):
        pass

    def on_generate_state_not_relaxed_impl(self, state : mm.State, action : mm.GroundAction, action_cost: float, problem : mm.Problem, pddl_repositories : mm.PDDLRepositories):
        pass

    def on_close_state_impl(self, state : mm.State, problem : mm.Problem, pddl_repositories : mm.PDDLRepositories):
        pass

    def on_finish_f_layer_impl(self, f_value: float, num_expanded_states : int, num_generated_states : int):
        pass

    def on_prune_state_impl(self, state : mm.State, problem : mm.Problem, pddl_repositories : mm.PDDLRepositories):
        pass

    def on_start_search_impl(self, start_state : mm.State, problem : mm.Problem, pddl_repositories : mm.PDDLRepositories):
        pass

    def on_end_search_impl(self, num_reached_fluent_atoms : int, num_reached_derived_atoms: int, num_bytes_for_unextended_state_portion: int, num_bytes_for_extended_state_portion: int, num_bytes_for_nodes: int, num_bytes_for_actions: int, num_bytes_for_axioms: int, num_states: int, num_nodes: int, num_actions: int, num_axioms: int):
        pass

    def on_solved_impl(self, ground_action_plan: List[mm.GroundAction], pddl_repositories: mm.PDDLRepositories):
        pass

    def on_unsolvable_impl(self):
        pass

    def on_exhausted_impl(self):
        pass

def test_astar_search():
    """ Test parsing a PDDL domain and problem file.
    """
    domain_filepath = str(ROOT_DIR / "data" / "gripper" / "domain.pddl")
    problem_filepath = str(ROOT_DIR / "data" / "gripper" / "test_problem.pddl")
    parser = mm.PDDLParser(domain_filepath, problem_filepath)
    grounder = mm.Grounder(parser.get_problem(), parser.get_pddl_repositories())
    applicable_action_generator = mm.LiftedApplicableActionGenerator(grounder.get_action_grounder())
    axiom_evaluator = mm.LiftedAxiomEvaluator(grounder.get_axiom_grounder())
    state_repository = mm.StateRepository(axiom_evaluator)

    blind_heuristic = CustomBlindHeuristic()
    goal_count_heuristic = CustomGoalCountHeuristic(parser.get_problem(), parser.get_pddl_repositories())

    event_handler = CustomAStarAlgorithmEventHandler(False)
    result = mm.find_solution_astar(applicable_action_generator, state_repository, goal_count_heuristic, None, event_handler)

    assert result.status == mm.SearchStatus.SOLVED
    assert len(result.plan) == 3
