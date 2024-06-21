from pymimir import FaithfulAbstraction

from pathlib import Path

ROOT_DIR = (Path(__file__).parent.parent.parent.parent).absolute()


def test_faithful_abstraction():
    """ Test the construction of faithful abstraction
    """
    domain_filepath = str(ROOT_DIR / "data" / "gripper" / "domain.pddl")
    problem_filepath = str(ROOT_DIR / "data" / "gripper" / "p-2-0.pddl")

    abstraction = FaithfulAbstraction.create(domain_filepath, problem_filepath)

    assert abstraction.get_num_states() == 12
    assert abstraction.get_num_transitions() == 46
    assert abstraction.get_num_goal_states() == 2
    assert abstraction.get_num_deadend_states() == 0


def test_faithful_abstraction_parallel():
    """ Test the parallel construction of parallel faithful abstraction
    """
    domain_filepath = str(ROOT_DIR / "data" / "gripper" / "domain.pddl")
    problem_filepath_1 = str(ROOT_DIR / "data" / "gripper" / "p-1-0.pddl")
    problem_filepath_2 = str(ROOT_DIR / "data" / "gripper" / "p-2-0.pddl")
    problem_filepaths = [problem_filepath_1, problem_filepath_2]

    abstractions = FaithfulAbstraction.create(domain_filepath, problem_filepaths)

    assert len(abstractions) == 2

    # Reduced from 8 to 6 abstract states.
    assert abstractions[0].get_num_states() == 6
    # Reduced from 28 to 12 abstract states.
    assert abstractions[1].get_num_states() == 12
