from pymimir import PDDLParser, LiftedAAG, SuccessorStateGenerator

from pathlib import Path

ROOT_DIR = (Path(__file__).parent.parent.parent.parent).absolute()


def test_ssg_ownership():
    """ Test correct implementation of ownership semantics of ssg.
    """
    domain_filepath = str(ROOT_DIR / "data" / "gripper" / "domain.pddl")
    problem_filepath = str(ROOT_DIR / "data" / "gripper" / "test_problem.pddl")
    pddl_parser = PDDLParser(domain_filepath, problem_filepath)

    ssg = SuccessorStateGenerator(LiftedAAG(pddl_parser.get_problem(), pddl_parser.get_factories()))
    initial_state = ssg.get_or_create_initial_state()

    del ssg

    assert initial_state.to_string(pddl_parser.get_problem(), pddl_parser.get_factories())
