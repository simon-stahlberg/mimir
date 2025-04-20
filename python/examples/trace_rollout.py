""" This example illustrates how to create traces string representations of actions and objects 
    that may be obtained from plans of external planners.
"""

import pymimir.advanced.search as search
import pymimir.advanced.formalism as formalism

from pathlib import Path
from typing import Dict

ROOT_DIR = (Path(__file__).parent.parent.parent).absolute()


def main():
    domain_filepath = str(ROOT_DIR / "data" / "gripper" / "domain.pddl")
    problem_filepath = str(ROOT_DIR / "data" / "gripper" / "test_problem.pddl")
    problem = formalism.Problem.create(domain_filepath, problem_filepath)

    actions_by_name: Dict[str, formalism.GroundAction] = dict()
    for action in problem.get_domain().get_actions():
        actions_by_name[action.get_name()] = action
    objects_by_name: Dict[str, formalism.Object] = dict()
    for object in problem.get_problem_and_domain_objects():
        objects_by_name[object.get_name()] = object

    applicable_action_generator = search.LiftedApplicableActionGenerator(problem)
    axiom_evaluator = search.LiftedAxiomEvaluator(problem)
    state_repository = search.StateRepository(axiom_evaluator)
    problem_goal_strategy = search.ProblemGoalStrategy(problem)

    # Empty static goal is trivially satisfied
    assert(problem_goal_strategy.test_static_goal())

    initial_state, initial_metric_value = state_repository.get_or_create_initial_state()
    print(initial_state.to_string(problem))
    assert(not problem_goal_strategy.test_dynamic_goal(initial_state))

    move_rooma_roomb_action = problem.ground(actions_by_name["move"], formalism.ObjectList([objects_by_name["rooma"], objects_by_name["roomb"]]))
    print(move_rooma_roomb_action.to_string(problem))

    successor_state, successor_metric_value = state_repository.get_or_create_successor_state(initial_state, move_rooma_roomb_action, initial_metric_value)
    print(successor_state.to_string(problem))
    assert(not problem_goal_strategy.test_dynamic_goal(successor_state))


if __name__ == "__main__":
    main()
