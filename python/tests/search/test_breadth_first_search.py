from pymimir import PDDLParser, LiftedApplicableActionGenerator, LiftedAxiomEvaluator, StateRepository, BrFSAlgorithm, SearchStatus

from pathlib import Path

ROOT_DIR = (Path(__file__).parent.parent.parent.parent).absolute()


def test_breadth_first_search():
    """ Test parsing a PDDL domain and problem file.
    """
    domain_filepath = str(ROOT_DIR / "data" / "gripper" / "domain.pddl")
    problem_filepath = str(ROOT_DIR / "data" / "gripper" / "test_problem.pddl")
    parser = PDDLParser(domain_filepath, problem_filepath)
    applicable_action_generator = LiftedApplicableActionGenerator(parser.get_problem(), parser.get_pddl_repositories())
    axiom_evaluator = LiftedAxiomEvaluator(parser.get_problem(), parser.get_pddl_repositories())
    state_repository = StateRepository(axiom_evaluator)
    breadth_first_search_algorithm = BrFSAlgorithm(applicable_action_generator, state_repository)
    result = breadth_first_search_algorithm.find_solution()

    assert result.status == SearchStatus.SOLVED
    assert len(result.plan) == 3
