import pymimir.advanced.formalism as formalism
import pymimir.advanced.search as search

from pathlib import Path

ROOT_DIR = (Path(__file__).parent.parent.parent.parent).absolute()


def test_applicable_action_generator_ownership():
    """ Test `ApplicableActionGenerator`.
    """
    domain_filepath = str(ROOT_DIR / "data" / "gripper" / "domain.pddl")
    problem_filepath = str(ROOT_DIR / "data" / "gripper" / "test_problem.pddl")
    problem = formalism.Problem.create(domain_filepath, problem_filepath, formalism.ParserOptions())

    applicable_action_generator = search.KPKCLiftedApplicableActionGenerator.create(problem)
    axiom_evaluator = search.KPKCLiftedAxiomEvaluator.create(problem)
    state_repository = search.StateRepository.create(axiom_evaluator)
    initial_state, initial_metric_value = state_repository.get_or_create_initial_state()
    actions = applicable_action_generator.generate_applicable_actions(initial_state)

    assert len(actions) == 6
    assert actions[0].to_string(problem)
