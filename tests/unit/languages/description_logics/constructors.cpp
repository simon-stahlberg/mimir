#include "mimir/languages/description_logics/evaluation_context.hpp"
#include "mimir/languages/description_logics/generator.hpp"
#include "mimir/languages/description_logics/grammar.hpp"

#include <gtest/gtest.h>

namespace mimir::tests
{

struct EvaluationContextWrapper
{
    dl::DenotationRepository<dl::Concept> concept_denotation_repository;
    dl::DenotationRepository<dl::Role> role_denotation_repository;
    size_t state_id;
    GroundAtomList<Static> static_state_atoms;
    GroundAtomList<Fluent> fluent_state_atoms;
    GroundAtomList<Derived> derived_state_atoms;
    GroundAtomList<Static> static_goal_atoms;
    GroundAtomList<Fluent> fluent_goal_atoms;
    GroundAtomList<Derived> derived_goal_atoms;

    dl::EvaluationContext evaluation_context;

    explicit EvaluationContextWrapper(size_t num_objects) :
        concept_denotation_repository(),
        role_denotation_repository(),
        state_id(),
        static_state_atoms(),
        fluent_state_atoms(),
        derived_state_atoms(),
        static_goal_atoms(),
        fluent_goal_atoms(),
        derived_goal_atoms(),
        evaluation_context(dl::EvaluationContext(concept_denotation_repository,
                                                 role_denotation_repository,
                                                 state_id,
                                                 static_state_atoms,
                                                 fluent_state_atoms,
                                                 derived_state_atoms,
                                                 static_goal_atoms,
                                                 fluent_goal_atoms,
                                                 derived_goal_atoms,
                                                 num_objects))
    {
    }
};

TEST(MimirTests, LanguagesDescriptionLogicsConstructorConceptPredicateStateTest)
{
    // Initialize the evaluation context
    auto num_objects = (size_t) 4;
    auto evaluation_context_wrapper = EvaluationContextWrapper(num_objects);
    auto& evaluation_context = evaluation_context_wrapper.evaluation_context;

    // Create some ground atoms
    auto pddl_factories = std::make_shared<PDDLFactories>();
    auto unary_predicate = pddl_factories->get_or_create_predicate<Fluent>(
        std::string("binary_predicate"),
        VariableList { pddl_factories->get_or_create_variable("arg0", 0), pddl_factories->get_or_create_variable("arg1", 0) });

    // TODO: Create a concept predicate state
    auto constructor_repositories = dl::ConstructorRepositories();

    // TODO: Evaluate it!
}

}