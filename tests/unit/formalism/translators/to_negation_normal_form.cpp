/*
 * Copyright (C) 2023 Dominik Drexler and Simon Stahlberg
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "mimir/formalism/parser.hpp"
#include "mimir/formalism/translators.hpp"

#include <gtest/gtest.h>
#include <loki/loki.hpp>

namespace mimir::tests
{

TEST(MimirTests, FormalismTranslatorsToNegationNormalForm)
{
    const auto domain_file = fs::path(std::string(DATA_DIR) + "miconic/domain.pddl");
    const auto problem_file = fs::path(std::string(DATA_DIR) + "miconic/test_problem.pddl");

    auto domain_parser = loki::DomainParser(domain_file);
    auto problem_parser = loki::ProblemParser(problem_file, domain_parser);

    auto domain = domain_parser.get_domain();
    auto problem = problem_parser.get_problem();

    // std::cout << "\nInput domain and problem" << std::endl;
    // std::cout << *domain << std::endl;
    // std::cout << *problem << std::endl;

    auto to_nnf_translator = ToNNFTranslator(domain_parser.get_repositories());
    auto translated_problem = to_nnf_translator.run(*problem);
    auto translated_domain = translated_problem->get_domain();

    // std::cout << "\nTranslated domain and problem" << std::endl;
    // std::cout << *translated_problem->get_domain() << std::endl;
    // std::cout << *translated_problem << std::endl;
}

}
