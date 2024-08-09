from pymimir import PDDLParser, LiftedApplicableActionGenerator, State, AStarAlgorithm, SearchStatus, IHeuristic

from pathlib import Path

ROOT_DIR = (Path(__file__).parent.parent.parent.parent).absolute()


class BlindHeuristic(IHeuristic):
    def __init__(self):
        IHeuristic.__init__(self)  # Without this, a TypeError is raised.

    def compute_heuristic(self, state : State) -> float:
        return 0.


def test_astar_search():
    """ Test parsing a PDDL domain and problem file.
    """
    domain_filepath = str(ROOT_DIR / "data" / "gripper" / "domain.pddl")
    problem_filepath = str(ROOT_DIR / "data" / "gripper" / "test_problem.pddl")
    parser = PDDLParser(domain_filepath, problem_filepath)
    lifted_applicable_action_generator = LiftedApplicableActionGenerator(parser.get_problem(), parser.get_pddl_factories())
    blind_heuristic = BlindHeuristic()
    astar_search_algorithm = AStarAlgorithm(lifted_applicable_action_generator, blind_heuristic)
    search_status, plan = astar_search_algorithm.find_solution()

    assert search_status == SearchStatus.SOLVED
    assert len(plan) == 3
