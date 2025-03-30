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

#include "mimir/search/generalized_search_context.hpp"

#include "mimir/formalism/problem.hpp"

using namespace mimir::formalism;

namespace mimir::search
{

GeneralizedSearchContext::GeneralizedSearchContext(const fs::path& domain_filepath,
                                                   const std::vector<fs::path>& problem_filepaths,
                                                   const SearchContext::Options& options) :
    GeneralizedSearchContext(GeneralizedProblem(domain_filepath, problem_filepaths), options)
{
}
GeneralizedSearchContext::GeneralizedSearchContext(const fs::path& domain_filepath, const fs::path& problems_directory, const SearchContext::Options& options) :
    GeneralizedSearchContext(GeneralizedProblem(domain_filepath, problems_directory), options)
{
}

GeneralizedSearchContext::GeneralizedSearchContext(GeneralizedProblem generalized_problem, const SearchContext::Options& options) :
    m_domain(generalized_problem.get_domain()),
    m_generalized_problem(std::move(generalized_problem)),
    m_search_contexts()
{
    for (const auto& problem : get_generalized_problem().get_problems())
    {
        m_search_contexts.emplace_back(problem, options);
    }
}

GeneralizedSearchContext::GeneralizedSearchContext(GeneralizedProblem generalized_problem, SearchContextList search_contexts) :
    m_domain(generalized_problem.get_domain()),
    m_generalized_problem(std::move(generalized_problem)),
    m_search_contexts(std::move(search_contexts))
{
    if (!all_of(m_search_contexts.begin(),
                m_search_contexts.end(),
                [this](auto&& arg) { return arg.get_problem()->get_domain() == get_generalized_problem().get_domain(); }))
    {
        throw std::runtime_error("GeneralizedSearchContext::GeneralizedSearchContext: Expected all given search contexts to be defined over the same domain.");
    }
}

const formalism::Domain& GeneralizedSearchContext::get_domain() const { return m_domain; }

const GeneralizedProblem& GeneralizedSearchContext::get_generalized_problem() const { return m_generalized_problem; }

const SearchContextList& GeneralizedSearchContext::get_search_contexts() const { return m_search_contexts; }

}
