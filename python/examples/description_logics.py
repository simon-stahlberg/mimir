""" This example illustrates the interface of the description logics component.
"""

import pymimir.advanced.search as search
import pymimir.advanced.datasets as datasets
import pymimir.advanced.languages.description_logics as description_logics

from pathlib import Path

ROOT_DIR = (Path(__file__).parent.parent.parent).absolute()


class MyVisitor(description_logics.ConstructorVisitor):
    """ Recursively visit the constructors. """
    def __init__(self):
        super().__init__()

    def visit(self, constructor):
        if isinstance(constructor, description_logics.ConceptBot):
            self.visit_concept_bot(constructor)
        elif isinstance(constructor, description_logics.ConceptTop):
            self.visit_concept_top(constructor)
        elif isinstance(constructor, description_logics.ConceptStaticAtomicState):
            self.visit_concept_static_atomic_state(constructor)
        elif isinstance(constructor, description_logics.ConceptFluentAtomicState):
            self.visit_concept_fluent_atomic_state(constructor)
        elif isinstance(constructor, description_logics.ConceptDerivedAtomicState):
            self.visit_concept_derived_atomic_state(constructor)
        elif isinstance(constructor, description_logics.ConceptStaticAtomicGoal):
            self.visit_concept_static_atomic_goal(constructor)
        elif isinstance(constructor, description_logics.ConceptFluentAtomicGoal):
            self.visit_concept_fluent_atomic_goal(constructor)
        elif isinstance(constructor, description_logics.ConceptDerivedAtomicGoal):
            self.visit_concept_derived_atomic_goal(constructor)
        elif isinstance(constructor, description_logics.ConceptIntersection):
            self.visit_concept_intersection(constructor)
        elif isinstance(constructor, description_logics.ConceptUnion):
            self.visit_concept_union(constructor)
        elif isinstance(constructor, description_logics.ConceptNegation):
            self.visit_concept_negation(constructor)
        elif isinstance(constructor, description_logics.ConceptValueRestriction):
            self.visit_concept_value_restriction(constructor)
        elif isinstance(constructor, description_logics.ConceptExistentialQuantification):
            self.visit_concept_existential_quantification(constructor)
        elif isinstance(constructor, description_logics.ConceptRoleValueMapContainment):
            self.visit_concept_role_value_map_containment(constructor)
        elif isinstance(constructor, description_logics.ConceptRoleValueMapEquality):
            self.visit_concept_role_value_map_equality(constructor)
        elif isinstance(constructor, description_logics.ConceptNominal):
            self.visit_concept_nominal(constructor)
        elif isinstance(constructor, description_logics.RoleUniversal):
            self.visit_role_universal(constructor)
        elif isinstance(constructor, description_logics.RoleStaticAtomicState):
            self.visit_role_static_atomic_state(constructor)
        elif isinstance(constructor, description_logics.RoleFluentAtomicState):
            self.visit_role_fluent_atomic_state(constructor)
        elif isinstance(constructor, description_logics.RoleDerivedAtomicState):
            self.visit_role_derived_atomic_state(constructor)
        elif isinstance(constructor, description_logics.RoleStaticAtomicGoal):
            self.visit_role_static_atomic_goal(constructor)
        elif isinstance(constructor, description_logics.RoleFluentAtomicGoal):
            self.visit_role_fluent_atomic_goal(constructor)
        elif isinstance(constructor, description_logics.RoleDerivedAtomicGoal):
            self.visit_role_derived_atomic_goal(constructor)
        elif isinstance(constructor, description_logics.RoleIntersection):
            self.visit_role_intersection(constructor)
        elif isinstance(constructor, description_logics.RoleUnion):
            self.visit_role_union(constructor)
        elif isinstance(constructor, description_logics.RoleComplement):
            self.visit_role_complement(constructor)
        elif isinstance(constructor, description_logics.RoleInverse):
            self.visit_role_inverse(constructor)
        elif isinstance(constructor, description_logics.RoleComposition):
            self.visit_role_composition(constructor)
        elif isinstance(constructor, description_logics.RoleTransitiveClosure):
            self.visit_role_transitive_closure(constructor)
        elif isinstance(constructor, description_logics.RoleReflexiveTransitiveClosure):
            self.visit_role_reflexive_transitive_closure(constructor)
        elif isinstance(constructor, description_logics.RoleRestriction):
            self.visit_role_restriction(constructor)
        elif isinstance(constructor, description_logics.RoleIdentity):
            self.visit_role_identity(constructor)
        elif isinstance(constructor, description_logics.BooleanStaticAtomicState):
            self.visit_boolean_static_atomic_state(constructor)
        elif isinstance(constructor, description_logics.BooleanFluentAtomicState):
            self.visit_boolean_fluent_atomic_state(constructor)
        elif isinstance(constructor, description_logics.BooleanDerivedAtomicState):
            self.visit_boolean_derived_atomic_state(constructor)
        elif isinstance(constructor, description_logics.BooleanConceptNonempty):
            self.visit_boolean_nonempty_concept(constructor)
        elif isinstance(constructor, description_logics.BooleanRoleNonempty):
            self.visit_boolean_nonempty_role(constructor)
        elif isinstance(constructor, description_logics.NumericalConceptCount):
            self.visit_numerical_count_concept(constructor)
        elif isinstance(constructor, description_logics.NumericalRoleCount):
            self.visit_numerical_count_role(constructor)
        elif isinstance(constructor, description_logics.NumericalDistance):
            self.visit_numerical_distance(constructor)
        else:
            raise NotImplementedError(f"No visit handler for: {type(constructor)}")

    def visit_concept_bot(self, constructor : description_logics.ConceptBot):
        pass
    def visit_concept_top(self, constructor : description_logics.ConceptTop):
        pass
    def visit_concept_static_atomic_state(self, constructor : description_logics.ConceptStaticAtomicState):
        pass
    def visit_concept_fluent_atomic_state(self, constructor : description_logics.ConceptFluentAtomicState): 
        pass
    def visit_concept_derived_atomic_state(self, constructor : description_logics.ConceptDerivedAtomicState): 
        pass
    def visit_concept_static_atomic_goal(self, constructor : description_logics.ConceptStaticAtomicGoal): 
        pass
    def visit_concept_fluent_atomic_goal(self, constructor : description_logics.ConceptFluentAtomicGoal): 
        pass
    def visit_concept_derived_atomic_goal(self, constructor : description_logics.ConceptDerivedAtomicGoal): 
        pass
    def visit_concept_intersection(self, constructor : description_logics.ConceptIntersection): 
        self.visit(constructor.get_left_concept())
        self.visit(constructor.get_right_concept())
    def visit_concept_union(self, constructor : description_logics.ConceptUnion): 
        self.visit(constructor.get_left_concept())
        self.visit(constructor.get_right_concept())
    def visit_concept_negation(self, constructor : description_logics.ConceptNegation): 
        self.visit(constructor.get_concept())
    def visit_concept_value_restriction(self, constructor : description_logics.ConceptValueRestriction): 
        self.visit(constructor.get_role())
        self.visit(constructor.get_concept())
    def visit_concept_existential_quantification(self, constructor : description_logics.ConceptExistentialQuantification): 
        self.visit(constructor.get_role())
        self.visit(constructor.get_concept())
    def visit_concept_role_value_map_containment(self, constructor : description_logics.ConceptRoleValueMapContainment): 
        self.visit(constructor.get_left_role())
        self.visit(constructor.get_right_role())
    def visit_concept_role_value_map_equality(self, constructor : description_logics.ConceptRoleValueMapEquality): 
        self.visit(constructor.get_left_role())
        self.visit(constructor.get_right_role())
    def visit_concept_nominal(self, constructor : description_logics.ConceptNominal): 
        pass
    def visit_role_universal(self, constructor : description_logics.RoleUniversal): 
        pass
    def visit_role_static_atomic_state(self, constructor : description_logics.RoleStaticAtomicState): 
        pass
    def visit_role_fluent_atomic_state(self, constructor : description_logics.RoleFluentAtomicState): 
        pass
    def visit_role_derived_atomic_state(self, constructor : description_logics.RoleDerivedAtomicState): 
        pass
    def visit_role_static_atomic_goal(self, constructor : description_logics.RoleStaticAtomicGoal): 
        pass
    def visit_role_fluent_atomic_goal(self, constructor : description_logics.RoleFluentAtomicGoal): 
        pass
    def visit_role_derived_atomic_goal(self, constructor : description_logics.RoleDerivedAtomicGoal): 
        pass
    def visit_role_intersection(self, constructor : description_logics.RoleIntersection):
        self.visit(constructor.get_left_role())
        self.visit(constructor.get_right_role())
    def visit_role_union(self, constructor : description_logics.RoleUnion): 
        self.visit(constructor.get_left_role())
        self.visit(constructor.get_right_role())
    def visit_role_complement(self, constructor : description_logics.RoleComplement): 
        self.visit(constructor.get_role())
    def visit_role_inverse(self, constructor : description_logics.RoleInverse): 
        self.visit(constructor.get_role())
    def visit_role_composition(self, constructor : description_logics.RoleComposition): 
        self.visit(constructor.get_left_role())
        self.visit(constructor.get_right_role())
    def visit_role_transitive_closure(self, constructor : description_logics.RoleTransitiveClosure): 
        self.visit(constructor.get_role())
    def visit_role_reflexive_transitive_closure(self, constructor : description_logics.RoleReflexiveTransitiveClosure): 
        self.visit(constructor.get_role())
    def visit_role_restriction(self, constructor : description_logics.RoleRestriction): 
        self.visit(constructor.get_role())
        self.visit(constructor.get_concept())
    def visit_role_identity(self, constructor : description_logics.RoleIdentity): 
        self.visit(constructor.get_concept())
    def visit_boolean_static_atomic_state(self, constructor : description_logics.BooleanStaticAtomicState): 
        pass
    def visit_boolean_fluent_atomic_state(self, constructor : description_logics.BooleanFluentAtomicState): 
        pass
    def visit_boolean_derived_atomic_state(self, constructor : description_logics.BooleanDerivedAtomicState): 
        pass
    def visit_boolean_nonempty_concept(self, constructor : description_logics.BooleanConceptNonempty): 
        self.visit(constructor.get_constructor())
    def visit_boolean_nonempty_role(self, constructor : description_logics.BooleanRoleNonempty): 
        self.visit(constructor.get_constructor())
    def visit_numerical_count_concept(self, constructor : description_logics.NumericalConceptCount): 
        self.visit(constructor.get_constructor())
    def visit_numerical_count_role(self, constructor : description_logics.NumericalRoleCount): 
        self.visit(constructor.get_constructor())
    def visit_numerical_distance(self, constructor : description_logics.NumericalDistance): 
        self.visit(constructor.get_left_concept())
        self.visit(constructor.get_role())
        self.visit(constructor.get_left_concept())


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
            # Check whether the sentence matches grammar specification
            assert(grammar.test_match(constructor))
            # Visit the constructor
            constructor.accept(MyVisitor())

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
            constructor.accept(MyVisitor())

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
            constructor.accept(MyVisitor())

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
            constructor.accept(MyVisitor())

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
