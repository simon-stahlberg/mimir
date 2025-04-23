""" This example illustrates the interface of the description logics component.
"""

import pymimir.advanced.search as search
import pymimir.advanced.datasets as datasets
import pymimir.advanced.languages.description_logics as description_logics
import pymimir.advanced.languages.general_policies as general_policies

from functools import singledispatchmethod
from pathlib import Path

from description_logics import MyConstructorVisitor

ROOT_DIR = (Path(__file__).parent.parent.parent).absolute()


class MyGeneralPolicyVisitor:
    """ Recursively visit the general policy. """
    def __init__(self):
        self.constructor_visitor = MyConstructorVisitor()

    @singledispatchmethod
    def visit(self, arg):
        raise NotImplementedError("Missing implementation for the constructor!" )
    
    @visit.register
    def _(self, feature : general_policies.NamedConcept):
        self.constructor_visitor.visit(feature.get_feature())
    @visit.register
    def _(self, feature : general_policies.NamedRole):
        self.constructor_visitor.visit(feature.get_feature())
    @visit.register
    def _(self, feature : general_policies.NamedBoolean):
        self.constructor_visitor.visit(feature.get_feature())
    @visit.register
    def _(self, feature : general_policies.NamedNumerical):
        self.constructor_visitor.visit(feature.get_feature())
    @visit.register
    def _(self, condition : general_policies.PositiveBooleanCondition):
        self.visit(condition.get_feature())
    @visit.register
    def _(self, condition : general_policies.NegativeBooleanCondition):
        self.visit(condition.get_feature())
    @visit.register
    def _(self, condition : general_policies.GreaterNumericalCondition):
        self.visit(condition.get_feature())
    @visit.register
    def _(self, condition : general_policies.EqualNumericalCondition):
        self.visit(condition.get_feature())
    @visit.register
    def _(self, effect : general_policies.PositiveBooleanEffect):
        self.visit(effect.get_feature())
    @visit.register
    def _(self, effect : general_policies.NegativeBooleanEffect):
        self.visit(effect.get_feature())
    @visit.register
    def _(self, effect : general_policies.UnchangedBooleanEffect):
        self.visit(effect.get_feature())
    @visit.register
    def _(self, effect : general_policies.IncreaseNumericalEffect):
        self.visit(effect.get_feature())
    @visit.register
    def _(self, effect : general_policies.DecreaseNumericalEffect):
        self.visit(effect.get_feature())
    @visit.register
    def _(self, effect : general_policies.UnchangedNumericalEffect):
        self.visit(effect.get_feature())
    @visit.register
    def _(self, rule : general_policies.Rule):
        for condition in rule.get_conditions():
            self.visit(condition)
        for effect in rule.get_effects():
            self.visit(effect)
    @visit.register
    def _(self, policy : general_policies.GeneralPolicy):
        for boolean in policy.get_boolean_features():
            self.visit(boolean)
        for numerical in policy.get_boolean_features():
            self.visit(numerical)
        for rule in policy.get_rules():
            self.visit(rule)


def main():
    domain_filepath = str(ROOT_DIR / "data" / "gripper" / "domain.pddl")
    problem_filepath = str(ROOT_DIR / "data" / "gripper" / "p-1-0.pddl")

    search_context_options = search.SearchContextOptions()
    search_context_options.mode = search.SearchMode.GROUNDED
    generalized_search_context = search.GeneralizedSearchContext.create(domain_filepath, [problem_filepath], search_context_options)

    state_space_options = datasets.StateSpaceOptions()
    state_space_options.symmetry_pruning = True 
    generalized_state_space_options = datasets.GeneralizedStateSpaceOptions()
    knowledge_base_options = datasets.KnowledgeBaseOptions()
    knowledge_base_options.state_space_options = state_space_options
    knowledge_base_options.generalized_state_space_options = generalized_state_space_options
    knowledge_base = datasets.KnowledgeBase.create(generalized_search_context, knowledge_base_options)

    repositories = general_policies.Repositories()
    dl_repositories = description_logics.Repositories()
    denotation_repositories = description_logics.DenotationRepositories()

    # Create a policy from description.
    general_policy = repositories.get_or_create_general_policy(
        """
        [boolean_features]
            <boolean_r_b> ::= 
                @boolean_nonempty 
                    @concept_existential_quantification 
                        @role_atomic_goal "at" true 
                        @concept_atomic_state "at-robby"

        [numerical_features]
            <numerical_c> ::= 
                @numerical_count 
                    @concept_existential_quantification 
                        @role_atomic_state "carry" 
                        @concept_top
            <numerical_b> ::=
                @numerical_count 
                    @concept_negation
                        @concept_role_value_map_equality
                            @role_atomic_state "at"
                            @role_atomic_goal "at" true

        [policy_rules]
            { @negative_boolean_condition <boolean_r_b>, @equal_numerical_condition <numerical_c>, @greater_numerical_condition <numerical_b> } 
            -> { @unchanged_boolean_effect <boolean_r_b>, @unchanged_numerical_effect <numerical_b>, @increase_numerical_effect <numerical_c> }
            { @positive_boolean_condition <boolean_r_b>, @equal_numerical_condition <numerical_c>, @greater_numerical_condition <numerical_b> } 
            -> { @negative_boolean_effect <boolean_r_b>, @unchanged_numerical_effect <numerical_b>, @unchanged_numerical_effect <numerical_c> }
            { @positive_boolean_condition <boolean_r_b>, @greater_numerical_condition <numerical_c>, @greater_numerical_condition <numerical_b> } 
            -> { @unchanged_boolean_effect <boolean_r_b>, @decrease_numerical_effect <numerical_c>, @decrease_numerical_effect <numerical_b> }
            { @negative_boolean_condition <boolean_r_b>, @greater_numerical_condition <numerical_c>, @greater_numerical_condition <numerical_b> } 
            -> { @positive_boolean_effect <boolean_r_b>, @unchanged_numerical_effect <numerical_b>, @unchanged_numerical_effect <numerical_c> }
        """,
        knowledge_base.get_domain(), dl_repositories)
    # Print the identical string representation of the policy.
    print(general_policy)
    # Visit the policy
    MyGeneralPolicyVisitor().visit(general_policy)
    # Test whether the policy is structurally terminating
    assert(general_policy.is_terminating(repositories))
    # Test whether the policy solves the generalized state space.
    assert(general_policy.solves(knowledge_base.get_generalized_state_space(), denotation_repositories) == general_policies.SolvabilityStatus.SOLVED)

    # Create the policy from above off-the-shelf.
    general_policy = general_policies.GeneralPolicyFactory.get_or_create_general_policy_gripper(knowledge_base.get_domain(), repositories, dl_repositories)
    # Print the identical string representation of the policy.
    print(general_policy)
    # Visit the policy
    MyGeneralPolicyVisitor().visit(general_policy)
    # Test whether the policy is structurally terminating
    assert(general_policy.is_terminating(repositories))
    # Test whether the policy solves the generalized state space.
    assert(general_policy.solves(knowledge_base.get_generalized_state_space(), denotation_repositories) == general_policies.SolvabilityStatus.SOLVED)

if __name__ == "__main__":
    main()
