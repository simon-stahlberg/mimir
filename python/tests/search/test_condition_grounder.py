import pymimir as mm

from pathlib import Path

ROOT_DIR = (Path(__file__).parent.parent.parent.parent).absolute()


# def test_condition_grounder():
#     """ Test implementation of `LiftedConjunctionGrounder`.
#     """
#     domain_filepath = str(ROOT_DIR / "data" / "gripper" / "domain.pddl")
#     problem_filepath = str(ROOT_DIR / "data" / "gripper" / "test_problem.pddl")
#     parser = mm.PDDLParser(domain_filepath, problem_filepath)

#     domain = parser.get_domain()
#     problem = parser.get_problem()
#     pddl_repositories = parser.get_pddl_repositories()
#     goal_variables, goal_lifted, goal_static_lifted = problem.get_fluent_goal_condition().lift(domain, pddl_repositories)
#     grounder = mm.Grounder(parser.get_problem(), parser.get_pddl_repositories())
#     existentially_quantified_conjunctive_condition = pddl_repositories.get_or_create_existentially_quantified_conjunctive_condition(goal_variables, mm.StaticLiteralList(), goal_lifted, mm.DerivedLiteralList())
#     condition_grounder = mm.SatisficingBindingGenerator(grounder.get_literal_grounder(), existentially_quantified_conjunctive_condition)
#     applicable_action_generator = mm.LiftedApplicableActionGenerator(grounder.get_action_grounder())
#     axiom_evaluator = mm.LiftedAxiomEvaluator(grounder.get_axiom_grounder())
#     state_repository = mm.StateRepository(axiom_evaluator)
#     initial_state = state_repository.get_or_create_initial_state()
#     goal_bindings = condition_grounder.generate_ground_conjunctions(initial_state, 1_000_000)

#     del state_repository
#     del applicable_action_generator
#     del condition_grounder

#     assert goal_bindings
#     assert len(goal_bindings) == 2


if __name__ == "__main__":
    # test_condition_grounder()
    pass
