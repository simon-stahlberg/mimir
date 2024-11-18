from pymimir import PDDLParser, LiftedApplicableActionGenerator, LiftedConjunctionGrounder, StateRepository, StaticLiteralList, DerivedLiteralList

from pathlib import Path

ROOT_DIR = (Path(__file__).parent.parent.parent.parent).absolute()


def test_lifted_conjunction_grounder():
    """ Test implementation of `LiftedConjunctionGrounder`.
    """
    domain_filepath = str(ROOT_DIR / "data" / "gripper" / "domain.pddl")
    problem_filepath = str(ROOT_DIR / "data" / "gripper" / "test_problem.pddl")
    pddl_parser = PDDLParser(domain_filepath, problem_filepath)

    problem = pddl_parser.get_problem()
    factories = pddl_parser.get_pddl_repositories()
    goal_variables, goal_lifted = problem.get_fluent_goal_condition().lift(factories)
    conjunction_grounder = LiftedConjunctionGrounder(problem, goal_variables, StaticLiteralList(), goal_lifted, DerivedLiteralList(), factories)
    applicable_action_generator = LiftedApplicableActionGenerator(pddl_parser.get_problem(), pddl_parser.get_pddl_repositories())
    state_repository = StateRepository(applicable_action_generator)
    initial_state = state_repository.get_or_create_initial_state()
    goal_bindings = conjunction_grounder.ground(initial_state)

    del state_repository
    del applicable_action_generator
    del conjunction_grounder

    assert goal_bindings
    assert len(goal_bindings) == 2


if __name__ == "__main__":
    test_lifted_conjunction_grounder()
