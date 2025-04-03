import pymimir.advanced.search as search
import pymimir.advanced.datasets as datasets

from pathlib import Path

ROOT_DIR = (Path(__file__).parent.parent.parent.parent).absolute()


def test_knowledge_base_without_symmetry_reduction():
    """ Test the construction of generalized state space
    """
    domain_filepath = str(ROOT_DIR / "data" / "gripper" / "domain.pddl")
    problem_filepath_1 = str(ROOT_DIR / "data" / "gripper" / "p-1-0.pddl")
    problem_filepath_2 = str(ROOT_DIR / "data" / "gripper" / "p-2-0.pddl")

    state_space_options = datasets.StateSpaceOptions()
    state_space_options.symmetry_pruning = False 

    generalized_state_space_options = datasets.GeneralizedStateSpaceOptions()
    generalized_state_space_options.symmetry_pruning = False

    knowledge_base_options = datasets.KnowledgeBaseOptions()
    knowledge_base_options.state_space_options = state_space_options
    knowledge_base_options.generalized_state_space_options = generalized_state_space_options

    generalized_search_context = search.GeneralizedSearchContext.create(domain_filepath, [problem_filepath_1, problem_filepath_2])

    knowledge_base = datasets.KnowledgeBase.create(generalized_search_context, knowledge_base_options)

    generalized_state_space = knowledge_base.get_generalized_state_space()

    class_graph = generalized_state_space.get_graph()
    assert class_graph.get_num_vertices() == 36
    assert class_graph.get_num_edges() == 128
    assert len(generalized_state_space.get_goal_vertices()) == 4
    assert len(generalized_state_space.get_unsolvable_vertices()) == 0

def test_knowledge_base_with_symmetry_reduction():
    """ Test the construction of generalized state space
    """
    domain_filepath = str(ROOT_DIR / "data" / "gripper" / "domain.pddl")
    problem_filepath_1 = str(ROOT_DIR / "data" / "gripper" / "p-1-0.pddl")
    problem_filepath_2 = str(ROOT_DIR / "data" / "gripper" / "p-2-0.pddl")

    state_space_options = datasets.StateSpaceOptions()
    state_space_options.symmetry_pruning = True 

    generalized_state_space_options = datasets.GeneralizedStateSpaceOptions()
    generalized_state_space_options.symmetry_pruning = True

    knowledge_base_options = datasets.KnowledgeBaseOptions()
    knowledge_base_options.state_space_options = state_space_options
    knowledge_base_options.generalized_state_space_options = generalized_state_space_options

    generalized_search_context = search.GeneralizedSearchContext.create(domain_filepath, [problem_filepath_1, problem_filepath_2])

    knowledge_base = datasets.KnowledgeBase.create(generalized_search_context, knowledge_base_options)

    generalized_state_space = knowledge_base.get_generalized_state_space()

    class_graph = generalized_state_space.get_graph()
    assert class_graph.get_num_vertices() == 18
    assert class_graph.get_num_edges() == 52
    assert len(generalized_state_space.get_goal_vertices()) == 4
    assert len(generalized_state_space.get_unsolvable_vertices()) == 0

