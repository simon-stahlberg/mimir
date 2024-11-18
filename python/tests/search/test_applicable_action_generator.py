from pymimir import PDDLParser, LiftedApplicableActionGenerator, StateRepository

from pathlib import Path

ROOT_DIR = (Path(__file__).parent.parent.parent.parent).absolute()


def test_applicable_action_generator_ownership():
    """ Test correct implementation of ownership semantics of `ApplicableActionGenerator`.
    """
    domain_filepath = str(ROOT_DIR / "data" / "gripper" / "domain.pddl")
    problem_filepath = str(ROOT_DIR / "data" / "gripper" / "test_problem.pddl")
    pddl_parser = PDDLParser(domain_filepath, problem_filepath)

    applicable_action_generator = LiftedApplicableActionGenerator(pddl_parser.get_problem(), pddl_parser.get_pddl_repositories())
    state_repository = StateRepository(applicable_action_generator)
    initial_state = state_repository.get_or_create_initial_state()
    actions = applicable_action_generator.compute_applicable_actions(initial_state)

    del state_repository
    del applicable_action_generator

    assert actions
    assert actions[0].to_string(pddl_parser.get_pddl_repositories())
