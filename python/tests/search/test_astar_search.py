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
    heuristic = search.MaxHeuristic(delete_relaxation)

    astar_options = search.AStarEagerOptions()

    result = search.find_solution_astar_eager(search_context, heuristic, astar_options)

    assert result.status == search.SearchStatus.SOLVED
    assert len(result.plan) == 3
