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

#include "mimir/formalism/problem_context.hpp"

#include "mimir/formalism/parser.hpp"

namespace mimir
{

ProblemContext::ProblemContext(const fs::path& domain_filepath, const fs::path& problem_filepath)
{
    auto parser = PDDLParser(domain_filepath, problem_filepath);
    m_problem = parser.get_problem();
    m_repositories = parser.get_pddl_repositories();
}

ProblemContext::ProblemContext(Problem problem, std::shared_ptr<PDDLRepositories> repositories) : m_problem(problem), m_repositories(std::move(repositories)) {}

std::vector<ProblemContext> ProblemContext::create(const fs::path& domain_filepath, const std::vector<fs::path>& problem_filepaths)
{
    auto result = ProblemContextList {};
    for (const auto& problem_filepath : problem_filepaths)
    {
        result.emplace_back(domain_filepath, problem_filepath);
    }
    return result;
}

std::vector<ProblemContext> ProblemContext::create(const fs::path& domain_filepath, const fs::path& problems_directory)
{
    auto result = ProblemContextList {};
    for (const auto& problem_filepath : fs::directory_iterator(problems_directory))
    {
        result.emplace_back(domain_filepath, problem_filepath);
    }
    return result;
}

Problem ProblemContext::get_problem() const { return m_problem; }

const std::shared_ptr<PDDLRepositories>& ProblemContext::get_repositories() const { return m_repositories; }
}
