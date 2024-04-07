#include "mimir/formalism/parser.hpp"
#include "mimir/formalism/translators.hpp"

#include <gtest/gtest.h>
#include <loki/parser.hpp>

namespace mimir::tests
{

TEST(MimirTests, FormalismTranslatorsToDisjunctiveNormalForm)
{
    const auto domain_file = fs::path(std::string(DATA_DIR) + "schedule/domain.pddl");
    const auto problem_file = fs::path(std::string(DATA_DIR) + "schedule/problem.pddl");

    auto domain_parser = loki::DomainParser(domain_file);
    auto problem_parser = loki::ProblemParser(problem_file, domain_parser);

    auto parser = PDDLParser(domain_file, problem_file);
    auto domain = parser.get_domain();
    auto problem = parser.get_problem();

    std::cout << "\nInput domain and problem" << std::endl;
    std::cout << *domain << std::endl;
    std::cout << *problem << std::endl;

    auto to_nnf_translator = ToNNFTranslator(parser.get_factories());
    auto to_dnf_translator = ToDNFTranslator(parser.get_factories(), to_nnf_translator);
    auto translated_problem = to_dnf_translator.run(*problem);
    auto translated_domain = translated_problem->get_domain();

    std::cout << "\nTranslated domain and problem" << std::endl;
    std::cout << *translated_problem->get_domain() << std::endl;
    std::cout << *translated_problem << std::endl;
}

}
