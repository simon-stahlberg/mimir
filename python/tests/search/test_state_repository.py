from pymimir import PDDLParser, LiftedApplicableActionGenerator, StateRepository

from pathlib import Path

ROOT_DIR = (Path(__file__).parent.parent.parent.parent).absolute()


def test_state_repository_ownership():
    """ Test correct implementation of ownership semantics of `StateRepository`.
    """
    domain_filepath = str(ROOT_DIR / "data" / "gripper" / "domain.pddl")
    problem_filepath = str(ROOT_DIR / "data" / "gripper" / "test_problem.pddl")
    pddl_parser = PDDLParser(domain_filepath, problem_filepath)

    state_repository = StateRepository(LiftedApplicableActionGenerator(pddl_parser.get_problem(), pddl_parser.get_pddl_factories()))
    initial_state = state_repository.get_or_create_initial_state()

    del state_repository

    assert initial_state.to_string(pddl_parser.get_problem(), pddl_parser.get_pddl_factories())
