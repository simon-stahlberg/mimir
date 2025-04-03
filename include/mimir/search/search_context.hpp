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

#ifndef MIMIR_FORMALISM_SEARCH_CONTEXT_HPP_
#define MIMIR_FORMALISM_SEARCH_CONTEXT_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/search/declarations.hpp"

namespace mimir::search
{

class SearchContextImpl
{
private:
    formalism::Problem m_problem;
    ApplicableActionGenerator m_applicable_action_generator;
    StateRepository m_state_repository;

    SearchContextImpl(formalism::Problem problem, ApplicableActionGenerator applicable_action_generator, StateRepository state_repository);

public:
    enum class SearchMode
    {
        GROUNDED = 0,
        LIFTED = 1
    };

    struct Options
    {
        SearchMode mode;

        Options() : mode(SearchMode::GROUNDED) {}
        explicit Options(SearchMode mode) : mode(mode) {}
    };

    /// @brief Construction from `ProblemImpl` construction API.
    /// @param domain_filepath
    /// @param problem_filepath
    /// @param options
    static SearchContext create(const fs::path& domain_filepath, const fs::path& problem_filepath, const Options& options = Options());

    /// @brief Construct for a given problem
    /// @param problem
    /// @param options
    static SearchContext create(formalism::Problem problem, const Options& options = Options());

    /// @brief Expert interface: fully customizable construction.
    /// @param problem
    /// @param applicable_action_generator
    /// @param state_repository
    static SearchContext create(formalism::Problem problem, ApplicableActionGenerator applicable_action_generator, StateRepository state_repository);

    const formalism::Problem& get_problem() const;
    const ApplicableActionGenerator get_applicable_action_generator() const;
    const StateRepository get_state_repository() const;
};
}

#endif
