from pymimir import StateSpace

from pathlib import Path

ROOT_DIR = (Path(__file__).parent.parent.parent.parent).absolute()


def test_state_space():
    """ Test the construction of state space
    """
    domain_filepath = str(ROOT_DIR / "data" / "gripper" / "domain.pddl")
    problem_filepath = str(ROOT_DIR / "data" / "gripper" / "p-2-0.pddl")

    state_space = StateSpace.create(domain_filepath, problem_filepath)

    assert state_space.get_num_states() == 28
    assert state_space.get_num_transitions() == 104
    assert state_space.get_num_goal_states() == 2
    assert state_space.get_num_deadend_states() == 0


def test_state_space_parallel():
    """ Test the parallel construction of state space
    """
    domain_filepath = str(ROOT_DIR / "data" / "gripper" / "domain.pddl")
    problem_filepath_1 = str(ROOT_DIR / "data" / "gripper" / "p-1-0.pddl")
    problem_filepath_2 = str(ROOT_DIR / "data" / "gripper" / "p-2-0.pddl")
    problem_filepaths = [problem_filepath_1, problem_filepath_2]

    state_spaces = StateSpace.create(domain_filepath, problem_filepaths)

    assert len(state_spaces) == 2
    assert state_spaces[0].get_num_states() == 8
    assert state_spaces[1].get_num_states() == 28
