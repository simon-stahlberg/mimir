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
#include "mimir/formalism/generalized_problem.hpp"
#include "mimir/search/declarations.hpp"
#include "mimir/search/search_context.hpp"

namespace mimir::search
{

class GeneralizedSearchContext
{
private:
    formalism::Domain m_domain;  ///< The common domain.

    formalism::GeneralizedProblem m_generalized_problem;

    SearchContextList m_search_contexts;

public:
    /// @brief Construction from `GeneralizedProblem` construction API.
    /// @param domain_filepath
    /// @param problem_filepaths
    /// @param options
    GeneralizedSearchContext(const fs::path& domain_filepath,
                             const std::vector<fs::path>& problem_filepaths,
                             const SearchContext::Options& options = SearchContext::Options());
    GeneralizedSearchContext(const fs::path& domain_filepath,
                             const fs::path& problems_directory,
                             const SearchContext::Options& options = SearchContext::Options());

    /// @brief Construction from a `GeneralizedProblem`
    /// @param problem
    /// @param options
    GeneralizedSearchContext(formalism::GeneralizedProblem generalized_problem, const SearchContext::Options& options = SearchContext::Options());

    /// @brief Expert interface: fully customizable construction.
    /// @param search_contexts
    GeneralizedSearchContext(formalism::GeneralizedProblem generalized_problem, SearchContextList search_contexts);

    const formalism::Domain& get_domain() const;
    const formalism::GeneralizedProblem& get_generalized_problem() const;
    const SearchContextList& get_search_contexts() const;
};
}

#endif
