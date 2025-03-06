import pymimir as mm

from pathlib import Path

ROOT_DIR = (Path(__file__).parent.parent.parent.parent).absolute()


def test_generalized_state_space_without_symmetry_reduction():
    """ Test the construction of generalized state space
    """
    domain_filepath = str(ROOT_DIR / "data" / "gripper" / "domain.pddl")
    problem_filepath_1 = str(ROOT_DIR / "data" / "gripper" / "p-1-0.pddl")
    problem_filepath_2 = str(ROOT_DIR / "data" / "gripper" / "p-2-0.pddl")

    generalized_search_context = mm.GeneralizedSearchContext(domain_filepath, [problem_filepath_1, problem_filepath_2])
    generalized_state_space = mm.GeneralizedStateSpace(generalized_search_context)

    class_graph = generalized_state_space.get_graph()
    assert class_graph.get_num_vertices() == 36
    assert class_graph.get_num_edges() == 128
    assert len(generalized_state_space.get_goal_vertices()) == 4
    assert len(generalized_state_space.get_unsolvable_vertices()) == 0

def test_generalized_state_space_with_symmetry_reduction():
    """ Test the construction of generalized state space
    """
    domain_filepath = str(ROOT_DIR / "data" / "gripper" / "domain.pddl")
    problem_filepath_1 = str(ROOT_DIR / "data" / "gripper" / "p-1-0.pddl")
    problem_filepath_2 = str(ROOT_DIR / "data" / "gripper" / "p-2-0.pddl")

    generalized_state_space_options = mm.GeneralizedStateSpaceOptions()
    generalized_state_space_options.problem_options.symmetry_pruning = True

    generalized_search_context = mm.GeneralizedSearchContext(domain_filepath, [problem_filepath_1, problem_filepath_2])
    generalized_state_space = mm.GeneralizedStateSpace(generalized_search_context, generalized_state_space_options)

    class_graph = generalized_state_space.get_graph()
    assert class_graph.get_num_vertices() == 18
    assert class_graph.get_num_edges() == 50
    assert len(generalized_state_space.get_goal_vertices()) == 4
    assert len(generalized_state_space.get_unsolvable_vertices()) == 0

