import pymimir as mm

from pathlib import Path

ROOT_DIR = (Path(__file__).parent.parent.parent.parent).absolute()


def test_applicable_action_generator_ownership():
    """ Test `ApplicableActionGenerator`.
    """
    domain_filepath = str(ROOT_DIR / "data" / "gripper" / "domain.pddl")
    problem_filepath = str(ROOT_DIR / "data" / "gripper" / "test_problem.pddl")
    problem = mm.Problem.create(domain_filepath, problem_filepath)

    applicable_action_generator = mm.LiftedApplicableActionGenerator(problem)
    axiom_evaluator = mm.LiftedAxiomEvaluator(problem)
    state_repository = mm.StateRepository(axiom_evaluator)
    initial_state = state_repository.get_or_create_initial_state()
    actions = applicable_action_generator.generate_applicable_actions(initial_state)

    assert len(actions) == 6
    assert actions[0].to_string(problem)
