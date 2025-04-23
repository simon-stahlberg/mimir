""" This example illustrates the interface of the description logics component.
"""

import pymimir.advanced.formalism as formalism
import pymimir.advanced.search as search
import pymimir.advanced.datasets as datasets
import pymimir.advanced.languages.description_logics as description_logics

from functools import singledispatchmethod
from pathlib import Path

ROOT_DIR = (Path(__file__).parent.parent.parent).absolute()

class MyVisitor(description_logics.ConstructorVisitor):
    def __init__(self):
        super().__init__()

    def visit(self, constructor : description_logics.ConceptBot):
        pass
    def visit(self, constructor : description_logics.ConceptTop):
        pass
    def visit(self, constructor : description_logics.ConceptStaticAtomicState):
        pass
    def visit(self, constructor : description_logics.ConceptFluentAtomicState): 
        pass
    def visit(self, constructor : description_logics.ConceptDerivedAtomicState): 
        pass
    def visit(self, constructor : description_logics.ConceptStaticAtomicGoal): 
        pass
    def visit(self, constructor : description_logics.ConceptFluentAtomicGoal): 
        pass
    def visit(self, constructor : description_logics.ConceptDerivedAtomicGoal): 
        pass
    def visit(self, constructor : description_logics.ConceptIntersection): 
        constructor.get_left_concept().accept(self)
        constructor.get_right_concept().accept(self)
    def visit(self, constructor : description_logics.ConceptUnion): 
        constructor.get_left_concept().accept(self)
        constructor.get_right_concept().accept(self)
    def visit(self, constructor : description_logics.ConceptNegation): 
        constructor.get_concept().accept(self)
    def visit(self, constructor : description_logics.ConceptValueRestriction): 
        constructor.get_role().accept(self)
        constructor.get_concept().accept(self)
    def visit(self, constructor : description_logics.ConceptExistentialQuantification): 
        constructor.get_role().accept(self)
        constructor.get_concept().accept(self)
    def visit(self, constructor : description_logics.ConceptRoleValueMapContainment): 
        constructor.get_left_role().accept(self)
        constructor.get_right_role().accept(self)
    def visit(self, constructor : description_logics.ConceptRoleValueMapEquality): 
        constructor.get_left_role().accept(self)
        constructor.get_right_role().accept(self)
    def visit(self, constructor : description_logics.ConceptNominal): 
        pass
    def visit(self, constructor : description_logics.RoleUniversal): 
        pass
    def visit(self, constructor : description_logics.RoleStaticAtomicState): 
        pass
    def visit(self, constructor : description_logics.RoleFluentAtomicState): 
        pass
    def visit(self, constructor : description_logics.RoleDerivedAtomicState): 
        pass
    def visit(self, constructor : description_logics.RoleStaticAtomicGoal): 
        pass
    def visit(self, constructor : description_logics.RoleFluentAtomicGoal): 
        pass
    def visit(self, constructor : description_logics.RoleDerivedAtomicGoal): 
        pass
    def visit(self, constructor : description_logics.RoleIntersection):
        constructor.get_left_role().accept(self)
        constructor.get_right_role().accept(self)
    def visit(self, constructor : description_logics.RoleUnion): 
        constructor.get_left_role().accept(self)
        constructor.get_right_role().accept(self)
    def visit(self, constructor : description_logics.RoleComplement): 
        constructor.get_role().accept(self)
    def visit(self, constructor : description_logics.RoleInverse): 
        constructor.get_role().accept(self)
    def visit(self, constructor : description_logics.RoleComposition): 
        constructor.get_left_role().accept(self)
        constructor.get_right_role().accept(self)
    def visit(self, constructor : description_logics.RoleTransitiveClosure): 
        constructor.get_role().accept(self)
    def visit(self, constructor : description_logics.RoleReflexiveTransitiveClosure): 
        constructor.get_role().accept(self)
    def visit(self, constructor : description_logics.RoleRestriction): 
        constructor.get_role().accept(self)
        constructor.get_concept().accept(self)
    def visit(self, constructor : description_logics.RoleIdentity): 
        constructor.get_concept().accept(self)
    def visit(self, constructor : description_logics.BooleanStaticAtomicState): 
        pass
    def visit(self, constructor : description_logics.BooleanFluentAtomicState): 
        pass
    def visit(self, constructor : description_logics.BooleanDerivedAtomicState): 
        pass
    def visit(self, constructor : description_logics.BooleanConceptNonempty): 
        constructor.get_constructor().accept(self)
    def visit(self, constructor : description_logics.BooleanRoleNonempty): 
        constructor.get_constructor().accept(self)
    def visit(self, constructor : description_logics.NumericalConceptCount): 
        constructor.get_constructor().accept(self)
    def visit(self, constructor : description_logics.NumericalRoleCount): 
        constructor.get_constructor().accept(self)
    def visit(self, constructor : description_logics.NumericalDistance): 
        constructor.get_left_concept().accept(self)
        constructor.get_role().accept(self)
        constructor.get_left_concept().accept(self)

def main():
    domain_filepath = str(ROOT_DIR / "data" / "gripper" / "domain.pddl")
    problem_filepath = str(ROOT_DIR / "data" / "gripper" / "p-1-0.pddl")

    # Create some states
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

    # Create a CNFGrammar for the domain.
    # You can also write down your own grammar in a BNF specification that will automatically be translated into a CNF representation.
    grammar = description_logics.CNFGrammar.create(description_logics.GrammarSpecificationEnum.FRANCES_ET_AL_AAAI2021, knowledge_base.get_domain())

    pruning_function = description_logics.StateListRefinementPruningFunction(knowledge_base.get_generalized_state_space())
    sentences = description_logics.GeneratedSentencesContainer()
    repositories = description_logics.Repositories()
    sentence_generator = description_logics.GeneratorVisitor(pruning_function, sentences, repositories, 5)
    sentence_generator.visit(grammar)

    print("================================================================================")
    print("Input Grammar in CNF")
    print("================================================================================")
    print(grammar)

    print("================================================================================")
    print("Generated concepts")
    print("================================================================================")
    for complexity, constructors in enumerate(sentences.get_concepts().get(grammar.get_concept_start_symbol())):
        print("Complexity:", complexity)
        for constructor in constructors:
            # Print the sentence
            print(f"    {constructor}")
            print(type(constructor))
            # Check whether the sentence matches grammar specification
            assert(grammar.test_match(constructor))
            # Visit the constructor (WIP)
            # constructor.accept(MyVisitor())

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
            # Visit the constructor (WIP)
            # constructor.accept(MyVisitor())

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
            # Visit the constructor (WIP)
            # constructor.accept(MyVisitor())

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
            # Visit the constructor (WIP)
            # constructor.accept(MyVisitor())

    print("================================================================================")
    print("Evaluate numericals")
    print("================================================================================")
    denotation_repositories = description_logics.DenotationRepositories()
    for complexity, constructors in enumerate(sentences.get_numericals().get(grammar.get_numerical_start_symbol())):
        print("Complexity:", complexity)
        for constructor in constructors:
            print(f"    {constructor}")
            for state_space in knowledge_base.get_state_spaces():
                for vertex in state_space.get_graph().get_vertices():
                    state = vertex.get_property_0()
                    problem = vertex.get_property_1()
                    context = description_logics.EvaluationContext(state, problem, denotation_repositories)
                    value = constructor.evaluate(context).get_data()
                    print("State:", state.to_string(problem))
                    print("Value:", value)
    
    

if __name__ == "__main__":
    main()
