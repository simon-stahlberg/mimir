import pymimir as mm

from pathlib import Path

ROOT_DIR = (Path(__file__).parent.parent.parent.parent).absolute()


def test_applicable_action_generator_ownership():
    """ Test correct implementation of ownership semantics of `ApplicableActionGenerator`.
    """
    domain_filepath = str(ROOT_DIR / "data" / "gripper" / "domain.pddl")
    problem_filepath = str(ROOT_DIR / "data" / "gripper" / "test_problem.pddl")
    pddl_parser = mm.PDDLParser(domain_filepath, problem_filepath)

    grounder = mm.Grounder(pddl_parser.get_problem(), pddl_parser.get_pddl_repositories())
    applicable_action_generator = mm.LiftedApplicableActionGenerator(grounder.get_action_grounder())
    applicable_action_generator_workspace = mm.ApplicableActionGeneratorWorkspace()
    axiom_evaluator = mm.LiftedAxiomEvaluator(grounder.get_axiom_grounder())
    state_repository = mm.StateRepository(axiom_evaluator)
    state_repository_workspace = mm.StateRepositoryWorkspace()
    initial_state = state_repository.get_or_create_initial_state(state_repository_workspace)
    actions = applicable_action_generator.generate_applicable_actions(initial_state, applicable_action_generator_workspace)

    del state_repository
    del applicable_action_generator

    assert actions
    assert actions[0].to_string(pddl_parser.get_pddl_repositories())
