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
#include "mimir/formalism/problem.hpp"
#include "mimir/formalism/translator.hpp"

namespace mimir::formalism
{

GeneralizedProblemImpl::GeneralizedProblemImpl(Domain domain, ProblemList problems) : m_domain(std::move(domain)), m_problems(std::move(problems))
{
    if (!all_of(m_problems.begin(), m_problems.end(), [this](auto&& arg) { return arg->get_domain() == m_domain; }))
    {
        throw std::runtime_error("GeneralizedProblemImpl::GeneralizedProblemImpl: Expected all given problems to be defined over the given domain.");
    }
}

GeneralizedProblem GeneralizedProblemImpl::create(const fs::path& domain_filepath, const std::vector<fs::path>& problem_filepaths, const loki::Options& options)
{
    auto parser = Parser(domain_filepath, options);
    auto translator = Translator(parser.get_domain());
    auto domain = translator.get_translated_domain();
    auto problems = ProblemList {};
    for (const auto& problem_filepath : problem_filepaths)
    {
        auto problem = parser.parse_problem(problem_filepath, options);
        problems.push_back(translator.translate(problem));
    }

    return create(std::move(domain), std::move(problems));
}

GeneralizedProblem GeneralizedProblemImpl::create(const fs::path& domain_filepath, const fs::path& problems_directory, const loki::Options& options)
{
    auto problem_filepaths = std::vector<fs::path> {};
    for (const auto& problem_filepath : fs::directory_iterator(problems_directory))
    {
        problem_filepaths.push_back(problem_filepath);
    }

    return create(domain_filepath, problem_filepaths);
}

GeneralizedProblem GeneralizedProblemImpl::create(Domain domain, ProblemList problems)
{
    return std::shared_ptr<GeneralizedProblemImpl>(new GeneralizedProblemImpl(std::move(domain), std::move(problems)));
}

const Domain& GeneralizedProblemImpl::get_domain() const { return m_domain; }
const ProblemList& GeneralizedProblemImpl::get_problems() const { return m_problems; }

}
