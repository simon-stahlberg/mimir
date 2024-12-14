import pymimir as mm

from pathlib import Path

ROOT_DIR = (Path(__file__).parent.parent.parent.parent).absolute()


def test_state_repository_ownership():
    """ Test correct implementation of ownership semantics of `StateRepository`.
    """
    domain_filepath = str(ROOT_DIR / "data" / "gripper" / "domain.pddl")
    problem_filepath = str(ROOT_DIR / "data" / "gripper" / "test_problem.pddl")
    parser = mm.PDDLParser(domain_filepath, problem_filepath)

    grounder = mm.Grounder(parser.get_problem(), parser.get_pddl_repositories())
    axiom_evaluator = mm.LiftedAxiomEvaluator(grounder.get_axiom_grounder())
    state_repository = mm.StateRepository(axiom_evaluator)
    state_repository_workspace = mm.StateRepositoryWorkspace()
    initial_state = state_repository.get_or_create_initial_state(state_repository_workspace)

    del state_repository

    assert initial_state.to_string(parser.get_problem(), parser.get_pddl_repositories())
