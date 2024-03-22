#include "mimir/formalism/parser.hpp"
#include "mimir/formalism/translators.hpp"

#include <gtest/gtest.h>
#include <loki/parser.hpp>

namespace mimir::tests
{

TEST(MimirTests, FormalismTranslatorsBase)
{
    const auto domain_file = fs::path(std::string(DATA_DIR) + "gripper/domain.pddl");
    const auto problem_file = fs::path(std::string(DATA_DIR) + "gripper/problem.pddl");
    auto domain_parser = loki::DomainParser(domain_file);
    auto problem_parser = loki::ProblemParser(problem_file, domain_parser);
    auto translated_pddl_factories = loki::PDDLFactories();
    auto translator = std::make_unique<TypeTranslator>(translated_pddl_factories);
    auto translated_problem = translator->run(*problem_parser.get_problem());
}

}
