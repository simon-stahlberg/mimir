import pymimir as mm

from pathlib import Path

ROOT_DIR = (Path(__file__).parent.parent.parent.parent).absolute()


def test_breadth_first_search():
    """ Test parsing a PDDL domain and problem file.
    """
    domain_filepath = str(ROOT_DIR / "data" / "gripper" / "domain.pddl")
    problem_filepath = str(ROOT_DIR / "data" / "gripper" / "test_problem.pddl")
    parser = mm.PDDLParser(domain_filepath, problem_filepath)
    grounder = mm.Grounder(parser.get_problem(), parser.get_pddl_repositories())
    applicable_action_generator = mm.LiftedApplicableActionGenerator(grounder.get_action_grounder())
    axiom_evaluator = mm.LiftedAxiomEvaluator(grounder.get_axiom_grounder())
    state_repository = mm.StateRepository(axiom_evaluator)
    result = mm.find_solution_brfs(applicable_action_generator, state_repository)

    assert result.status == mm.SearchStatus.SOLVED
    assert len(result.plan) == 3
