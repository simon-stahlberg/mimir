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

#ifndef MIMIR_SEARCH_ALGORITHMS_SIW_EVENT_HANDLERS_MINIMAL_HPP_
#define MIMIR_SEARCH_ALGORITHMS_SIW_EVENT_HANDLERS_MINIMAL_HPP_

#include "mimir/search/algorithms/siw/event_handlers/interface.hpp"

#include <iostream>

namespace mimir
{

/**
 * Implementation class
 */
class DefaultSIWAlgorithmEventHandler : public SIWAlgorithmEventHandlerBase<DefaultSIWAlgorithmEventHandler>
{
private:
    /* Implement SIWAlgorithmEventHandlerBase interface */
    friend class SIWAlgorithmEventHandlerBase<DefaultSIWAlgorithmEventHandler>;

    void on_start_search_impl(const Problem problem, const State initial_state, const PDDLFactories& pddl_factories) const;

    void on_start_subproblem_search_impl(const Problem problem, const State initial_state, const PDDLFactories& pddl_factories) const;

    void on_end_subproblem_search_impl(const IWAlgorithmStatistics& iw_statistics) const;

    void on_end_search_impl() const;

    void on_solved_impl(const GroundActionList& ground_action_plan) const;

    void on_unsolvable_impl() const;

    void on_exhausted_impl() const;

public:
    explicit DefaultSIWAlgorithmEventHandler(bool quiet = true);
};

}

#endif
