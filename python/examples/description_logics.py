""" This example illustrates the interface of the description logics component.
"""

import pymimir.advanced.search as search
import pymimir.advanced.datasets as datasets
import pymimir.advanced.languages.description_logics as description_logics

from functools import singledispatchmethod
from pathlib import Path

ROOT_DIR = (Path(__file__).parent.parent.parent).absolute()


class MyConstructorVisitor():
    """ Recursively visit the constructors. """
    def __init__(self):
        pass

    @singledispatchmethod
    def visit(self, arg):
        raise NotImplementedError("Missing implementation for the constructor!")

    @visit.register
    def _(self, constructor : description_logics.ConceptBot):
        pass
    @visit.register
    def _(self, constructor : description_logics.ConceptTop):
        pass
    @visit.register
    def _(self, constructor : description_logics.ConceptStaticAtomicState):
        pass
    @visit.register
    def _(self, constructor : description_logics.ConceptFluentAtomicState): 
        pass
    @visit.register
    def _(self, constructor : description_logics.ConceptDerivedAtomicState): 
        pass
    @visit.register
    def _(self, constructor : description_logics.ConceptStaticAtomicGoal): 
        pass
    @visit.register
    def _(self, constructor : description_logics.ConceptFluentAtomicGoal): 
        pass
    @visit.register
    def _(self, constructor : description_logics.ConceptDerivedAtomicGoal): 
        pass
    @visit.register
    def _(self, constructor : description_logics.ConceptIntersection): 
        self.visit(constructor.get_left_concept())
        self.visit(constructor.get_right_concept())
    @visit.register
    def _(self, constructor : description_logics.ConceptUnion): 
        self.visit(constructor.get_left_concept())
        self.visit(constructor.get_right_concept())
    @visit.register
    def _(self, constructor : description_logics.ConceptNegation): 
        self.visit(constructor.get_concept())
    @visit.register
    def _(self, constructor : description_logics.ConceptValueRestriction): 
        self.visit(constructor.get_role())
        self.visit(constructor.get_concept())
    @visit.register
    def _(self, constructor : description_logics.ConceptExistentialQuantification): 
        self.visit(constructor.get_role())
        self.visit(constructor.get_concept())
    @visit.register
    def _(self, constructor : description_logics.ConceptRoleValueMapContainment): 
        self.visit(constructor.get_left_role())
        self.visit(constructor.get_right_role())
    @visit.register
    def _(self, constructor : description_logics.ConceptRoleValueMapEquality): 
        self.visit(constructor.get_left_role())
        self.visit(constructor.get_right_role())
    @visit.register
    def _(self, constructor : description_logics.ConceptNominal): 
        pass
    @visit.register
    def _(self, constructor : description_logics.RoleUniversal): 
        pass
    @visit.register
    def _(self, constructor : description_logics.RoleStaticAtomicState): 
        pass
    @visit.register
    def _(self, constructor : description_logics.RoleFluentAtomicState): 
        pass
    @visit.register
    def _(self, constructor : description_logics.RoleDerivedAtomicState): 
        pass
    @visit.register
    def _(self, constructor : description_logics.RoleStaticAtomicGoal): 
        pass
    @visit.register
    def _(self, constructor : description_logics.RoleFluentAtomicGoal): 
        pass
    @visit.register
    def _(self, constructor : description_logics.RoleDerivedAtomicGoal): 
        pass
    @visit.register
    def _(self, constructor : description_logics.RoleIntersection):
        self.visit(constructor.get_left_role())
        self.visit(constructor.get_right_role())
    @visit.register
    def _(self, constructor : description_logics.RoleUnion): 
        self.visit(constructor.get_left_role())
        self.visit(constructor.get_right_role())
    @visit.register
    def _(self, constructor : description_logics.RoleComplement): 
        self.visit(constructor.get_role())
    @visit.register
    def _(self, constructor : description_logics.RoleInverse): 
        self.visit(constructor.get_role())
    @visit.register
    def _(self, constructor : description_logics.RoleComposition): 
        self.visit(constructor.get_left_role())
        self.visit(constructor.get_right_role())
    @visit.register
    def _(self, constructor : description_logics.RoleTransitiveClosure): 
        self.visit(constructor.get_role())
    @visit.register
    def _(self, constructor : description_logics.RoleReflexiveTransitiveClosure): 
        self.visit(constructor.get_role())
    @visit.register
    def _(self, constructor : description_logics.RoleRestriction): 
        self.visit(constructor.get_role())
        self.visit(constructor.get_concept())
    @visit.register
    def _(self, constructor : description_logics.RoleIdentity): 
        self.visit(constructor.get_concept())
    @visit.register
    def _(self, constructor : description_logics.BooleanStaticAtomicState): 
        pass
    @visit.register
    def _(self, constructor : description_logics.BooleanFluentAtomicState): 
        pass
    @visit.register
    def _(self, constructor : description_logics.BooleanDerivedAtomicState): 
        pass
    @visit.register
    def _(self, constructor : description_logics.BooleanConceptNonempty): 
        self.visit(constructor.get_constructor())
    @visit.register
    def _(self, constructor : description_logics.BooleanRoleNonempty): 
        self.visit(constructor.get_constructor())
    @visit.register
    def _(self, constructor : description_logics.NumericalConceptCount): 
        self.visit(constructor.get_constructor())
    @visit.register
    def _(self, constructor : description_logics.NumericalRoleCount): 
        self.visit(constructor.get_constructor())
    @visit.register
    def _(self, constructor : description_logics.NumericalDistance): 
        self.visit(constructor.get_left_concept())
        self.visit(constructor.get_role())
        self.visit(constructor.get_left_concept())


class MyCNFGrammarVisitor:
    """ Recursively visit the CNF grammar. 
        This can be useful to write a custom sentence generator.
        An available example implementation is `description_logics.GeneratorVisitor`.
    """
    def __init__(self):
        pass

    @singledispatchmethod
    def visit(self, arg):
        raise NotImplementedError("Missing implementation for the constructor!")
    
    @visit.register
    def _(self, constructor : description_logics.CNFConceptBot):
        pass
    @visit.register
    def _(self, constructor : description_logics.CNFConceptTop):
        pass
    @visit.register
    def _(self, constructor : description_logics.CNFConceptStaticAtomicState):
        pass
    @visit.register
    def _(self, constructor : description_logics.CNFConceptFluentAtomicState): 
        pass
    @visit.register
    def _(self, constructor : description_logics.CNFConceptDerivedAtomicState): 
        pass
    @visit.register
    def _(self, constructor : description_logics.CNFConceptStaticAtomicGoal): 
        pass
    @visit.register
    def _(self, constructor : description_logics.CNFConceptFluentAtomicGoal): 
        pass
    @visit.register
    def _(self, constructor : description_logics.CNFConceptDerivedAtomicGoal): 
        pass
    @visit.register
    def _(self, constructor : description_logics.CNFConceptIntersection): 
        self.visit(constructor.get_left_concept())
        self.visit(constructor.get_right_concept())
    @visit.register
    def _(self, constructor : description_logics.CNFConceptUnion): 
        self.visit(constructor.get_left_concept())
        self.visit(constructor.get_right_concept())
    @visit.register
    def _(self, constructor : description_logics.CNFConceptNegation): 
        self.visit(constructor.get_concept())
    @visit.register
    def _(self, constructor : description_logics.CNFConceptValueRestriction): 
        self.visit(constructor.get_role())
        self.visit(constructor.get_concept())
    @visit.register
    def _(self, constructor : description_logics.CNFConceptExistentialQuantification): 
        self.visit(constructor.get_role())
        self.visit(constructor.get_concept())
    @visit.register
    def _(self, constructor : description_logics.CNFConceptRoleValueMapContainment): 
        self.visit(constructor.get_left_role())
        self.visit(constructor.get_right_role())
    @visit.register
    def _(self, constructor : description_logics.CNFConceptRoleValueMapEquality): 
        self.visit(constructor.get_left_role())
        self.visit(constructor.get_right_role())
    @visit.register
    def _(self, constructor : description_logics.CNFConceptNominal): 
        pass
    @visit.register
    def _(self, constructor : description_logics.CNFRoleUniversal): 
        pass
    @visit.register
    def _(self, constructor : description_logics.CNFRoleStaticAtomicState): 
        pass
    @visit.register
    def _(self, constructor : description_logics.CNFRoleFluentAtomicState): 
        pass
    @visit.register
    def _(self, constructor : description_logics.CNFRoleDerivedAtomicState): 
        pass
    @visit.register
    def _(self, constructor : description_logics.CNFRoleStaticAtomicGoal): 
        pass
    @visit.register
    def _(self, constructor : description_logics.CNFRoleFluentAtomicGoal): 
        pass
    @visit.register
    def _(self, constructor : description_logics.CNFRoleDerivedAtomicGoal): 
        pass
    @visit.register
    def _(self, constructor : description_logics.CNFRoleIntersection):
        self.visit(constructor.get_left_role())
        self.visit(constructor.get_right_role())
    @visit.register
    def _(self, constructor : description_logics.CNFRoleUnion): 
        self.visit(constructor.get_left_role())
        self.visit(constructor.get_right_role())
    @visit.register
    def _(self, constructor : description_logics.CNFRoleComplement): 
        self.visit(constructor.get_role())
    @visit.register
    def _(self, constructor : description_logics.CNFRoleInverse): 
        self.visit(constructor.get_role())
    @visit.register
    def _(self, constructor : description_logics.CNFRoleComposition): 
        self.visit(constructor.get_left_role())
        self.visit(constructor.get_right_role())
    @visit.register
    def _(self, constructor : description_logics.CNFRoleTransitiveClosure): 
        self.visit(constructor.get_role())
    @visit.register
    def _(self, constructor : description_logics.CNFRoleReflexiveTransitiveClosure): 
        self.visit(constructor.get_role())
    @visit.register
    def _(self, constructor : description_logics.CNFRoleRestriction): 
        self.visit(constructor.get_role())
        self.visit(constructor.get_concept())
    @visit.register
    def _(self, constructor : description_logics.CNFRoleIdentity): 
        self.visit(constructor.get_concept())
    @visit.register
    def _(self, constructor : description_logics.CNFBooleanStaticAtomicState): 
        pass
    @visit.register
    def _(self, constructor : description_logics.CNFBooleanFluentAtomicState): 
        pass
    @visit.register
    def _(self, constructor : description_logics.CNFBooleanDerivedAtomicState): 
        pass
    @visit.register
    def _(self, constructor : description_logics.CNFBooleanConceptNonempty): 
        self.visit(constructor.get_constructor())
    @visit.register
    def _(self, constructor : description_logics.CNFBooleanRoleNonempty): 
        self.visit(constructor.get_constructor())
    @visit.register
    def _(self, constructor : description_logics.CNFNumericalConceptCount): 
        self.visit(constructor.get_constructor())
    @visit.register
    def _(self, constructor : description_logics.CNFNumericalRoleCount): 
        self.visit(constructor.get_constructor())
    @visit.register
    def _(self, constructor : description_logics.CNFNumericalDistance): 
        self.visit(constructor.get_left_concept())
        self.visit(constructor.get_role())
        self.visit(constructor.get_left_concept())
    @visit.register
    def _(self, nonterminal : description_logics.CNFConceptNonTerminal): 
        pass
    @visit.register
    def _(self, nonterminal : description_logics.CNFRoleNonTerminal): 
        pass
    @visit.register
    def _(self, nonterminal : description_logics.CNFBooleanNonTerminal): 
        pass
    @visit.register
    def _(self, nonterminal : description_logics.CNFNumericalNonTerminal): 
        pass
    @visit.register
    def _(self, rule : description_logics.CNFConceptDerivationRule): 
        self.visit(rule.get_head())
        self.visit(rule.get_body())
    @visit.register
    def _(self, rule : description_logics.CNFRoleDerivationRule): 
        self.visit(rule.get_head())
        self.visit(rule.get_body())
    @visit.register
    def _(self, rule : description_logics.CNFBooleanDerivationRule): 
        self.visit(rule.get_head())
        self.visit(rule.get_body())
    @visit.register
    def _(self, rule : description_logics.CNFNumericalDerivationRule): 
        self.visit(rule.get_head())
        self.visit(rule.get_body())
    @visit.register
    def _(self, rule : description_logics.CNFConceptSubstitutionRule): 
        self.visit(rule.get_head())
        self.visit(rule.get_body())
    @visit.register
    def _(self, rule : description_logics.CNFRoleSubstitutionRule): 
        self.visit(rule.get_head())
        self.visit(rule.get_body())
    @visit.register
    def _(self, rule : description_logics.CNFBooleanSubstitutionRule): 
        self.visit(rule.get_head())
        self.visit(rule.get_body())
    @visit.register
    def _(self, rule : description_logics.CNFNumericalSubstitutionRule): 
        self.visit(rule.get_head())
        self.visit(rule.get_body())
    @visit.register
    def _(self, grammar : description_logics.CNFGrammar): 
        for symbol in [grammar.get_concept_start_symbol()] + [grammar.get_role_start_symbol()] + [grammar.get_boolean_start_symbol()] + [grammar.get_numerical_start_symbol()]:
            if symbol is not None:
                self.visit(symbol)
        for rule in list(grammar.get_concept_derivation_rules()) + list(grammar.get_role_derivation_rules()) + list(grammar.get_boolean_derivation_rules()) + list(grammar.get_numerical_derivation_rules()):
            self.visit(rule)
        for rule in list(grammar.get_concept_substitution_rules()) + list(grammar.get_role_substitution_rules()) + list(grammar.get_boolean_substitution_rules()) + list(grammar.get_numerical_substitution_rules()):
            self.visit(rule)


def main():
    domain_filepath = str(ROOT_DIR / "data" / "gripper" / "domain.pddl")
    problem_filepath = str(ROOT_DIR / "data" / "gripper" / "p-1-0.pddl")

    # Create GeneralizedSearchContext
    search_context_options = search.SearchContextOptions()
    search_context_options.mode = search.SearchMode.GROUNDED
    generalized_search_context = search.GeneralizedSearchContext.create(domain_filepath, [problem_filepath], search_context_options)

    # Create KnowledgeBase
    state_space_options = datasets.StateSpaceOptions()
    state_space_options.symmetry_pruning = True 
    generalized_state_space_options = datasets.GeneralizedStateSpaceOptions()
    knowledge_base_options = datasets.KnowledgeBaseOptions()
    knowledge_base_options.state_space_options = state_space_options
    knowledge_base_options.generalized_state_space_options = generalized_state_space_options
    knowledge_base = datasets.KnowledgeBase.create(generalized_search_context, knowledge_base_options)

    # Create a CNFGrammar for the domain.
    # You can also write down your own grammar in a BNF specification that will automatically be translated into a CNF representation.
    grammar = description_logics.CNFGrammar.create(description_logics.GrammarSpecificationEnum.FRANCES_ET_AL_AAAI2021, knowledge_base.get_domain())

    denotation_repositories = description_logics.DenotationRepositories()
    pruning_function = description_logics.StateListRefinementPruningFunction(knowledge_base.get_generalized_state_space(), denotation_repositories)
    sentences = description_logics.GeneratedSentencesContainer()
    repositories = description_logics.Repositories()
    sentence_generator = description_logics.GeneratorVisitor(pruning_function, sentences, repositories, 5)
    sentence_generator.visit(grammar)

    print("================================================================================")
    print("Input Grammar in CNF")
    print("================================================================================")
    print(grammar)
    # Visit the grammar
    MyCNFGrammarVisitor().visit(grammar)

    print("================================================================================")
    print("Generated concepts")
    print("================================================================================")
    for complexity, constructors in enumerate(sentences.get_concepts().get(grammar.get_concept_start_symbol())):
        print("Complexity:", complexity)
        for constructor in constructors:
            # Print the sentence
            print(f"    {constructor}")
            # Check whether the sentence matches grammar specification
            assert(grammar.test_match(constructor))
            # Visit the constructor
            MyConstructorVisitor().visit(constructor)

    print("================================================================================")
    print("Generated roles")
    print("================================================================================")
    for complexity, constructors in enumerate(sentences.get_roles().get(grammar.get_role_start_symbol())):
        print("Complexity:", complexity)
        for constructor in constructors:
            # Print the sentence
            print(f"    {constructor}")
            # Check whether the sentence matches grammar specification
            assert(grammar.test_match(constructor))
            # Visit the constructor
            MyConstructorVisitor().visit(constructor)

    print("================================================================================")
    print("Generated booleans")
    print("================================================================================")
    for complexity, constructors in enumerate(sentences.get_booleans().get(grammar.get_boolean_start_symbol())):
        print("Complexity:", complexity)
        for constructor in constructors:
            # Print the sentence
            print(f"    {constructor}")
            # Check whether the sentence matches grammar specification
            assert(grammar.test_match(constructor))
            # Visit the constructor
            MyConstructorVisitor().visit(constructor)

    print("================================================================================")
    print("Generated numericals")
    print("================================================================================")
    for complexity, constructors in enumerate(sentences.get_numericals().get(grammar.get_numerical_start_symbol())):
        print("Complexity:", complexity)
        for constructor in constructors:
            # Print the sentence
            print(f"    {constructor}")
            # Check whether the sentence matches grammar specification
            assert(grammar.test_match(constructor))
            # Visit the constructor
            MyConstructorVisitor().visit(constructor)

    print("================================================================================")
    print("Evaluate numericals")
    print("================================================================================")
    
    for complexity, constructors in enumerate(sentences.get_numericals().get(grammar.get_numerical_start_symbol())):
        print("Complexity:", complexity)
        for constructor in constructors:
            print(f"    {constructor}")
            for state_space in knowledge_base.get_state_spaces():
                for vertex in state_space.get_graph().get_vertices():
                    state = datasets.get_state(vertex)
                    problem = datasets.get_problem(vertex)
                    context = description_logics.EvaluationContext(state, problem, denotation_repositories)
                    value = constructor.evaluate(context).get_data()
                    print("State:", state.to_string(problem))
                    print("Value:", value)
    
    

if __name__ == "__main__":
    main()
