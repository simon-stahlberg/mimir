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

#include "mimir/formalism/generalized_problem.hpp"
#include "mimir/formalism/problem.hpp"

using namespace mimir::formalism;

namespace mimir::search
{

GeneralizedSearchContextImpl::GeneralizedSearchContextImpl(formalism::Domain domain,
                                                           formalism::GeneralizedProblem generalized_problem,
                                                           SearchContextList search_contexts) :
    m_domain(std::move(domain)),
    m_generalized_problem(std::move(generalized_problem)),
    m_search_contexts(std::move(search_contexts))
{
}

GeneralizedSearchContext
GeneralizedSearchContextImpl::create(const fs::path& domain_filepath, const std::vector<fs::path>& problem_filepaths, const SearchContextImpl::Options& options)
{
    return create(GeneralizedProblemImpl::create(domain_filepath, problem_filepaths), options);
}

GeneralizedSearchContext
GeneralizedSearchContextImpl::create(const fs::path& domain_filepath, const fs::path& problems_directory, const SearchContextImpl::Options& options)
{
    return create(GeneralizedProblemImpl::create(domain_filepath, problems_directory), options);
}

GeneralizedSearchContext GeneralizedSearchContextImpl::create(GeneralizedProblem generalized_problem, const SearchContextImpl::Options& options)
{
    auto search_contexts = SearchContextList {};
    for (const auto& problem : generalized_problem->get_problems())
    {
        search_contexts.push_back(SearchContextImpl::create(problem, options));
    }

    return create(generalized_problem, search_contexts);
}

GeneralizedSearchContext GeneralizedSearchContextImpl::create(GeneralizedProblem generalized_problem, SearchContextList search_contexts)
{
    if (!all_of(search_contexts.begin(),
                search_contexts.end(),
                [&](auto&& arg) { return arg->get_problem()->get_domain() == generalized_problem->get_domain(); }))
    {
        throw std::runtime_error(
            "GeneralizedSearchContextImpl::GeneralizedSearchContextImpl: Expected all given search contexts to be defined over the same domain.");
    }

    return std::shared_ptr<GeneralizedSearchContextImpl>(
        new GeneralizedSearchContextImpl(generalized_problem->get_domain(), generalized_problem, search_contexts));
}

const formalism::Domain& GeneralizedSearchContextImpl::get_domain() const { return m_domain; }

const GeneralizedProblem& GeneralizedSearchContextImpl::get_generalized_problem() const { return m_generalized_problem; }

const SearchContextList& GeneralizedSearchContextImpl::get_search_contexts() const { return m_search_contexts; }

}
