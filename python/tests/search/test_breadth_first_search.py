import pymimir.advanced.search as search

from pathlib import Path

ROOT_DIR = (Path(__file__).parent.parent.parent.parent).absolute()


def test_breadth_first_search():
    """ Test parsing a PDDL domain and problem file.
    """
    domain_filepath = str(ROOT_DIR / "data" / "gripper" / "domain.pddl")
    problem_filepath = str(ROOT_DIR / "data" / "gripper" / "test_problem.pddl")
    search_context = search.SearchContext.create(domain_filepath, problem_filepath)
    result = search.find_solution_brfs(search_context)

    assert result.status == search.SearchStatus.SOLVED
    assert len(result.plan) == 3
