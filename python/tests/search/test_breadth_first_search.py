from pymimir import PDDLParser, LiftedApplicableActionGenerator, BrFSAlgorithm, SearchStatus

from pathlib import Path

ROOT_DIR = (Path(__file__).parent.parent.parent.parent).absolute()


def test_breadth_first_search():
    """ Test parsing a PDDL domain and problem file.
    """
    domain_filepath = str(ROOT_DIR / "data" / "gripper" / "domain.pddl")
    problem_filepath = str(ROOT_DIR / "data" / "gripper" / "test_problem.pddl")
    parser = PDDLParser(domain_filepath, problem_filepath)
    lifted_applicable_action_generator = LiftedApplicableActionGenerator(parser.get_problem(), parser.get_pddl_repositories())
    breadth_first_search_algorithm = BrFSAlgorithm(lifted_applicable_action_generator)
    search_status, plan = breadth_first_search_algorithm.find_solution()

    assert search_status == SearchStatus.SOLVED
    assert len(plan) == 3
