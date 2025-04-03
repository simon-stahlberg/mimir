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

#ifndef MIMIR_FORMALISM_GENERALIZED_SEARCH_CONTEXT_HPP_
#define MIMIR_FORMALISM_GENERALIZED_SEARCH_CONTEXT_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/search/declarations.hpp"
#include "mimir/search/search_context.hpp"

namespace mimir::search
{

class GeneralizedSearchContextImpl
{
private:
    formalism::Domain m_domain;

    formalism::GeneralizedProblem m_generalized_problem;

    SearchContextList m_search_contexts;

    GeneralizedSearchContextImpl(formalism::Domain domain, formalism::GeneralizedProblem generalized_problem, SearchContextList search_contexts);

public:
    /// @brief Construction from `GeneralizedProblem` construction API.
    /// @param domain_filepath
    /// @param problem_filepaths
    /// @param options
    static GeneralizedSearchContext create(const fs::path& domain_filepath,
                                           const std::vector<fs::path>& problem_filepaths,
                                           const SearchContextImpl::Options& options = SearchContextImpl::Options());
    static GeneralizedSearchContext
    create(const fs::path& domain_filepath, const fs::path& problems_directory, const SearchContextImpl::Options& options = SearchContextImpl::Options());

    /// @brief Construction from a `GeneralizedProblem`
    /// @param problem
    /// @param options
    static GeneralizedSearchContext create(formalism::GeneralizedProblem generalized_problem,
                                           const SearchContextImpl::Options& options = SearchContextImpl::Options());

    /// @brief Expert interface: fully customizable construction.
    /// @param search_contexts
    static GeneralizedSearchContext create(formalism::GeneralizedProblem generalized_problem, SearchContextList search_contexts);

    const formalism::Domain& get_domain() const;
    const formalism::GeneralizedProblem& get_generalized_problem() const;
    const SearchContextList& get_search_contexts() const;
};
}

#endif
