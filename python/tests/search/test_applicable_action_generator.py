import pymimir.advanced.formalism as formalism
import pymimir.advanced.search as search
import pymimir

from pathlib import Path

ROOT_DIR = (Path(__file__).parent.parent.parent.parent).absolute()


def test_applicable_action_generator_ownership():
    """ Test `ApplicableActionGenerator`.
    """
    domain_filepath = str(ROOT_DIR / "data" / "gripper" / "domain.pddl")
    problem_filepath = str(ROOT_DIR / "data" / "gripper" / "test_problem.pddl")
    problem = formalism.Problem.create(domain_filepath, problem_filepath, formalism.ParserOptions())

    applicable_action_generator = search.KPKCLiftedApplicableActionGenerator.create(problem, search.LiftedKPKCOptions())
    axiom_evaluator = search.KPKCLiftedAxiomEvaluator.create(problem)
    state_repository = search.StateRepository.create(axiom_evaluator)
    initial_state, initial_metric_value = state_repository.get_or_create_initial_state()
    actions = applicable_action_generator.generate_applicable_actions(initial_state)

    assert len(actions) == 6
    assert actions[0].to_string(problem)


def test_grounded_novelty_applicable_action_generator_bindings():
    domain_content = """
    (define (domain grounded-novelty-test)
        (:requirements :strips)
        (:predicates (p ?x) (q ?x))
        (:action make-q
            :parameters (?x)
            :precondition (and (p ?x))
            :effect (and (q ?x))))
    """
    problem_content = """
    (define (problem grounded-novelty-problem)
        (:domain grounded-novelty-test)
        (:objects a)
        (:init (p a))
        (:goal (q a)))
    """

    domain = pymimir.Domain(domain_content)
    problem = pymimir.Problem(domain, problem_content)
    novelty_generator = problem.create_grounded_novelty_applicable_action_generator()
    initial_state = problem.get_initial_state()

    applicable_actions = novelty_generator.generate_applicable_actions(initial_state)

    assert len(applicable_actions) == 1
    assert applicable_actions[0].get_action().get_name() == "make-q"

    successor_state = applicable_actions[0].apply(initial_state)
    novelty_generator.generate_applicable_actions(successor_state)
    assert novelty_generator.generate_applicable_actions(initial_state) == []

    novelty_generator.reset_novelty_history()

    reset_actions = novelty_generator.generate_applicable_actions(initial_state)
    assert len(reset_actions) == 1
    assert reset_actions[0].get_action().get_name() == "make-q"
