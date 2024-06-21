from pymimir import GlobalFaithfulAbstraction

from pathlib import Path

ROOT_DIR = (Path(__file__).parent.parent.parent.parent).absolute()


def test_global_faithful_abstraction():
    """ Test the parallel construction of parallel faithful abstraction
    """
    domain_filepath = str(ROOT_DIR / "data" / "gripper" / "domain.pddl")
    problem_filepath_1 = str(ROOT_DIR / "data" / "gripper" / "p-1-0.pddl")
    problem_filepath_2 = str(ROOT_DIR / "data" / "gripper" / "p-1-0.pddl")
    problem_filepath_3 = str(ROOT_DIR / "data" / "gripper" / "p-2-0.pddl")
    problem_filepaths = [problem_filepath_1, problem_filepath_2, problem_filepath_3]

    abstractions = GlobalFaithfulAbstraction.create(domain_filepath, problem_filepaths)

    # Problem 1 was pruned because it has 0 global non isomorphic states.
    assert len(abstractions) == 2

    # Reduced from 8 to 6 abstract states.
    abstraction_0 = abstractions[0]
    assert abstraction_0.get_num_states() == 6
    assert abstraction_0.get_num_isomorphic_states() == 0
    assert abstraction_0.get_num_non_isomorphic_states() == 6
    # Reduced from 28 to 12 abstract states.
    abstraction_1 = abstractions[1]
    assert abstraction_1.get_num_states() == 12
    assert abstraction_1.get_num_isomorphic_states() == 0
    assert abstraction_1.get_num_non_isomorphic_states() == 12
