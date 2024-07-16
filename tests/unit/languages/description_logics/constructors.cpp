#include "mimir/formalism/parser.hpp"
#include "mimir/languages/description_logics/generator.hpp"
#include "mimir/languages/description_logics/grammar.hpp"
#include "mimir/search/applicable_action_generators/lifted.hpp"
#include "mimir/search/successor_state_generator.hpp"

#include <gtest/gtest.h>

namespace mimir::tests
{

TEST(MimirTests, LanguagesDescriptionLogicsConstructorConceptPredicateStateTest)
{
    auto pddl_factories = std::make_shared<PDDLFactories>();
    auto requirements = pddl_factories->get_or_create_requirements(loki::RequirementEnumSet { loki::RequirementEnum::STRIPS });
    auto unary_predicate = pddl_factories->get_or_create_predicate<Fluent>(
        std::string("binary_predicate"),
        VariableList { pddl_factories->get_or_create_variable("arg0", 0), pddl_factories->get_or_create_variable("arg1", 0) });

    auto state_builder = StateBuilder();

    auto constructor_repositories = dl::ConstructorRepositories();
}

}