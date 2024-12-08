from pymimir import PDDLParser, LiftedApplicableActionGenerator, LiftedAxiomEvaluator, StaticGroundAtomList, StaticAssignmentSet, ConditionGrounder, StateRepository, StaticLiteralList, DerivedLiteralList

from pathlib import Path

ROOT_DIR = (Path(__file__).parent.parent.parent.parent).absolute()


def test_condition_grounder():
    """ Test implementation of `LiftedConjunctionGrounder`.
    """
    domain_filepath = str(ROOT_DIR / "data" / "gripper" / "domain.pddl")
    problem_filepath = str(ROOT_DIR / "data" / "gripper" / "test_problem.pddl")
    pddl_parser = PDDLParser(domain_filepath, problem_filepath)

    problem = pddl_parser.get_problem()
    pddl_repositories = pddl_parser.get_pddl_repositories()
    goal_variables, goal_lifted = problem.get_fluent_goal_condition().lift(pddl_repositories)
    static_initial_atoms = StaticGroundAtomList([literal.get_atom() for literal in problem.get_static_initial_literals()])
    static_assignment_set = StaticAssignmentSet(problem, problem.get_domain().get_static_predicates(), static_initial_atoms)
    condition_grounder = ConditionGrounder(problem, pddl_repositories, goal_variables, StaticLiteralList(), goal_lifted, DerivedLiteralList(), static_assignment_set)
    applicable_action_generator = LiftedApplicableActionGenerator(problem, pddl_repositories)
    axiom_evaluator = LiftedAxiomEvaluator(problem, pddl_repositories)
    state_repository = StateRepository(axiom_evaluator)
    initial_state = state_repository.get_or_create_initial_state()
    goal_bindings = condition_grounder.create_ground_conjunction_generator(initial_state, 1_000_000)

    del state_repository
    del applicable_action_generator
    del condition_grounder

    assert goal_bindings
    assert len(goal_bindings) == 2


if __name__ == "__main__":
    test_condition_grounder()
