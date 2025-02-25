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

#include "mimir/formalism/generalized_problem.hpp"

#include "mimir/formalism/parser.hpp"
#include "mimir/formalism/translator.hpp"

namespace mimir
{

GeneralizedProblem::GeneralizedProblem(const fs::path& domain_filepath, const std::vector<fs::path>& problem_filepaths, const loki::Options& options) :
    m_problems()
{
    auto parser = Parser(domain_filepath, options);
    auto domain_translation_result = translate(parser.get_domain());
    for (const auto& problem_filepath : problem_filepaths)
    {
        auto problem = parser.parse_problem(problem_filepath, options);
        m_problems.push_back(translate(problem, domain_translation_result));
    }
}

GeneralizedProblem::GeneralizedProblem(const fs::path& domain_filepath, const fs::path& problems_directory, const loki::Options& options)
{
    auto parser = Parser(domain_filepath, options);
    auto domain_translation_result = translate(parser.get_domain());
    for (const auto& problem_filepath : fs::directory_iterator(problems_directory))
    {
        auto problem = parser.parse_problem(problem_filepath, options);
        m_problems.push_back(translate(problem, domain_translation_result));
    }
}

const ProblemList& GeneralizedProblem::get_problems() const { return m_problems; }

}
