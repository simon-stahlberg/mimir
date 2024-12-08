from pymimir import PDDLParser, PDDLRepositories, Problem, LiftedApplicableActionGenerator, LiftedAxiomEvaluator, State, StateRepository, GroundAction, AStarAlgorithm, SearchStatus, IHeuristic, DefaultAStarAlgorithmEventHandler, AStarAlgorithmEventHandlerBase

from pathlib import Path
from typing import List

ROOT_DIR = (Path(__file__).parent.parent.parent.parent).absolute()


class CustomBlindHeuristic(IHeuristic):
    def __init__(self):
        IHeuristic.__init__(self)  # Without this, a TypeError is raised.

    def compute_heuristic(self, state : State) -> float:
        return 0.


class CustomGoalCountHeuristic(IHeuristic):
    def __init__(self, problem : Problem, pddl_repositories: PDDLRepositories):
        IHeuristic.__init__(self)  # Without this, a TypeError is raised.
        self.problem = problem
        self.pddl_repositories = pddl_repositories

        self.num_goal_literals = len(self.problem.get_fluent_goal_condition()) + len(self.problem.get_derived_goal_condition())

    def compute_heuristic(self, state : State) -> float:
        num_satisfied_goal_literals = 0
        for literal in self.problem.get_fluent_goal_condition():
            if state.literal_holds(literal):
                num_satisfied_goal_literals += 1
        for literal in self.problem.get_derived_goal_condition():
            if state.literal_holds(literal):
                num_satisfied_goal_literals += 1

        return self.num_goal_literals - num_satisfied_goal_literals


class CustomAStarAlgorithmEventHandler(AStarAlgorithmEventHandlerBase):
    def __init__(self, quiet = True):
        """
        AStarAlgorithmEventHandlerBase collects statistics and
        calls callback implementations only if quiet = False.
        """
        AStarAlgorithmEventHandlerBase.__init__(self, quiet)  # Without this, a TypeError is raised.

    def on_expand_state_impl(self, state : State, problem : Problem, pddl_repositories : PDDLRepositories):
        pass

    def on_generate_state_impl(self, state : State, action : GroundAction, action_cost: float, problem : Problem, pddl_repositories : PDDLRepositories):
        pass

    def on_generate_state_relaxed_impl(self, state : State, action : GroundAction, action_cost: float, problem : Problem, pddl_repositories : PDDLRepositories):
        pass

    def on_generate_state_not_relaxed_impl(self, state : State, action : GroundAction, action_cost: float, problem : Problem, pddl_repositories : PDDLRepositories):
        pass

    def on_close_state_impl(self, state : State, problem : Problem, pddl_repositories : PDDLRepositories):
        pass

    def on_finish_f_layer_impl(self, f_value: float, num_expanded_states : int, num_generated_states : int):
        pass

    def on_prune_state_impl(self, state : State, problem : Problem, pddl_repositories : PDDLRepositories):
        pass

    def on_start_search_impl(self, start_state : State, problem : Problem, pddl_repositories : PDDLRepositories):
        pass

    def on_end_search_impl(self):
        pass

    def on_solved_impl(self, ground_action_plan: List[GroundAction], pddl_repositories: PDDLRepositories):
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
    parser = PDDLParser(domain_filepath, problem_filepath)
    applicable_action_generator = LiftedApplicableActionGenerator(parser.get_problem(), parser.get_pddl_repositories())
    axiom_evaluator = LiftedAxiomEvaluator(parser.get_problem(), parser.get_pddl_repositories())
    state_repository = StateRepository(axiom_evaluator)

    blind_heuristic = CustomBlindHeuristic()
    goal_count_heuristic = CustomGoalCountHeuristic(parser.get_problem(), parser.get_pddl_repositories())

    event_handler = CustomAStarAlgorithmEventHandler(False)
    astar_search_algorithm = AStarAlgorithm(applicable_action_generator, state_repository, goal_count_heuristic, event_handler)
    result = astar_search_algorithm.find_solution()

    assert result.status == SearchStatus.SOLVED
    assert len(result.plan) == 3
