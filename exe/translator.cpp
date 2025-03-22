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

#include <iostream>
#include <mimir/formalism/domain.hpp>
#include <mimir/formalism/parser.hpp>
#include <mimir/formalism/problem.hpp>
#include <mimir/formalism/translator.hpp>

using namespace mimir::formalism;

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        std::cout << "Usage: ./loki <domain:str> <problem:str>" << std::endl;
        return 1;
    }
    const auto domain_file = std::string { argv[1] };
    const auto problem_file = std::string { argv[2] };

    auto options = loki::Options();
    options.quiet = false;
    options.strict = true;

    // 1. Parse the domain
    auto parser = Parser(domain_file, options);
    const auto domain = parser.get_domain();
    std::cout << *domain << std::endl << std::endl;

    // 2. Parse the problem
    const auto problem = parser.parse_problem(problem_file, options);
    std::cout << *problem << std::endl;

    // 3. Translate the domain
    auto translator = Translator(domain);
    std::cout << *translator.get_translated_domain() << std::endl;

    // 4. Translate the problem, throws an error if the problem is not defined over the original domain.
    const auto translated_problem = translator.translate(problem);
    std::cout << *translated_problem << std::endl;

    return 0;
}
