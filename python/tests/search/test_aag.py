from pymimir import PDDLParser, LiftedAAG, SuccessorStateGenerator

from pathlib import Path

ROOT_DIR = (Path(__file__).parent.parent.parent.parent).absolute()


def test_aag_ownership():
    """ Test correct implementation of ownership semantics of aag.
    """
    domain_filepath = str(ROOT_DIR / "data" / "gripper" / "domain.pddl")
    problem_filepath = str(ROOT_DIR / "data" / "gripper" / "test_problem.pddl")
    pddl_parser = PDDLParser(domain_filepath, problem_filepath)

    aag = LiftedAAG(pddl_parser.get_problem(), pddl_parser.get_factories())
    ssg = SuccessorStateGenerator(aag)
    initial_state = ssg.get_or_create_initial_state()
    actions = aag.compute_applicable_actions(initial_state)

    del ssg
    del aag

    assert actions
    assert actions[0].to_string(pddl_parser.get_factories())