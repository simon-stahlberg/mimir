#include "mimir/formalism/parser.hpp"
#include "mimir/formalism/translators.hpp"

#include <gtest/gtest.h>
#include <loki/parser.hpp>

namespace mimir::tests
{

TEST(MimirTests, FormalismTranslatorsBase)
{
    const auto domain_file = fs::path(std::string(DATA_DIR) + "miconic/domain.pddl");
    const auto problem_file = fs::path(std::string(DATA_DIR) + "miconic/problem.pddl");
    auto domain_parser = loki::DomainParser(domain_file);
    auto problem_parser = loki::ProblemParser(problem_file, domain_parser);

    auto translated_pddl_factories = loki::PDDLFactories();
    auto translator = TypeTranslator(translated_pddl_factories);
    auto translated_problem = translator.run(*problem_parser.get_problem());

    std::cout << "\nInput domain and problem" << std::endl;
    std::cout << *domain_parser.get_domain() << std::endl;
    std::cout << *problem_parser.get_problem() << std::endl;

    std::cout << "\nTranslated domain and problem" << std::endl;
    std::cout << *translated_problem->get_domain() << std::endl;
    std::cout << *translated_problem << std::endl;
}

}
