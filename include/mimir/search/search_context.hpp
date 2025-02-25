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

namespace mimir
{

class SearchContext
{
private:
    Problem m_problem;
    ApplicableActionGenerator m_applicable_action_generator;
    StateRepository m_state_repository;

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

    SearchContext(Problem problem, const Options& options = Options());
    SearchContext(Problem problem, ApplicableActionGenerator applicable_action_generator, StateRepository state_repository);

    static std::vector<SearchContext> create(const ProblemList& problems, const Options& options = Options());

    const Problem& get_problem() const;
    const ApplicableActionGenerator get_applicable_action_generator() const;
    const StateRepository get_state_repository() const;
};

using SearchContextList = std::vector<SearchContext>;
}

#endif
