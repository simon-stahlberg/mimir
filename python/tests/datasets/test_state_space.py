from pymimir import StateSpace

from pathlib import Path

ROOT_DIR = (Path(__file__).parent.parent.parent.parent).absolute()


def test_state_space():
    """ Test the construction of state space
    """
    domain_filepath = str(ROOT_DIR / "data" / "gripper" / "domain.pddl")
    problem_filepath = str(ROOT_DIR / "data" / "gripper" / "test_problem.pddl")

    state_space = StateSpace.create(domain_filepath, problem_filepath, 10000, 10000)

    assert state_space.get_num_states() == 28
    assert state_space.get_num_transitions() == 104
    assert state_space.get_num_goal_states() == 8
    assert state_space.get_num_deadend_states() == 0
