import pymimir.advanced.formalism as formalism
import pymimir.advanced.search as search

from pathlib import Path

ROOT_DIR = (Path(__file__).parent.parent.parent.parent).absolute()



def test_astar_search():
    """ Test parsing a PDDL domain and problem file.
    """
    domain_filepath = str(ROOT_DIR / "data" / "gripper" / "domain.pddl")
    problem_filepath = str(ROOT_DIR / "data" / "gripper" / "test_problem.pddl")
    search_context = search.SearchContext.create(domain_filepath, problem_filepath, search.SearchContextOptions())

    delete_relaxation = search.DeleteRelaxedProblemExplorator(search_context.get_problem())
    heuristic = search.MaxHeuristic.create(delete_relaxation)

    astar_options = search.AStarEagerOptions()

    result = search.find_solution_astar_eager(search_context, heuristic, astar_options)

    assert result.status == search.SearchStatus.SOLVED
    assert len(result.plan) == 3


def test_astar_search_2():
    """
    """
    domain_filepath = str(ROOT_DIR / "data" / "blocks_4" / "domain.pddl")
    problem_filepath = str(ROOT_DIR / "data" / "blocks_4" / "test_problem.pddl")

    parser = formalism.Parser(domain_filepath, formalism.ParserOptions())
    domain = parser.get_domain()
    problem = parser.parse_problem(problem_filepath, formalism.ParserOptions())

    axiom_evaluator = search.LiftedAxiomEvaluator.create(problem)
    state_repository = search.StateRepository.create(axiom_evaluator)
    applicable_action_generator = search.LiftedApplicableActionGenerator.create(problem)

    search_context = search.SearchContext.create(problem, applicable_action_generator, state_repository)

    heuristic = search.BlindHeuristic.create(problem)

    astar_options = search.AStarEagerOptions()

    result = search.find_solution_astar_eager(search_context, heuristic, astar_options)

    assert result.status == search.SearchStatus.SOLVED
    assert len(result.plan) == 4
