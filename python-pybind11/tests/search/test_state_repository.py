import pymimir as mm

from pathlib import Path

ROOT_DIR = (Path(__file__).parent.parent.parent.parent).absolute()


def test_state_repository_ownership():
    """ Test correct implementation of ownership semantics of `StateRepository`.
    """
    domain_filepath = str(ROOT_DIR / "data" / "gripper" / "domain.pddl")
    problem_filepath = str(ROOT_DIR / "data" / "gripper" / "test_problem.pddl")
    problem = mm.Problem.create(domain_filepath, problem_filepath)
    axiom_evaluator = mm.LiftedAxiomEvaluator(problem)
    state_repository = mm.StateRepository(axiom_evaluator)
    initial_state = state_repository.get_or_create_initial_state()

    del state_repository

    assert initial_state.to_string(problem)
