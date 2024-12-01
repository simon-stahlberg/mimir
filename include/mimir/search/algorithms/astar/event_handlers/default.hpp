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

#ifndef MIMIR_SEARCH_ALGORITHMS_ASTAR_EVENT_HANDLERS_MINIMAL_HPP_
#define MIMIR_SEARCH_ALGORITHMS_ASTAR_EVENT_HANDLERS_MINIMAL_HPP_

#include "mimir/search/algorithms/astar/event_handlers/interface.hpp"

namespace mimir
{

/**
 * Implementation class
 */
class DefaultAStarAlgorithmEventHandler : public StaticAStarAlgorithmEventHandlerBase<DefaultAStarAlgorithmEventHandler>
{
private:
    /* Implement StaticAStarAlgorithmEventHandlerBase interface */
    friend class StaticAStarAlgorithmEventHandlerBase<DefaultAStarAlgorithmEventHandler>;

    void on_expand_state_impl(State state, Problem problem, const PDDLRepositories& pddl_repositories) const;

    void on_generate_state_impl(State state, GroundAction action, ContinuousCost action_cost, Problem problem, const PDDLRepositories& pddl_repositories) const;

    void on_generate_state_relaxed_impl(State state,
                                        GroundAction action,
                                        ContinuousCost action_cost,
                                        Problem problem,
                                        const PDDLRepositories& pddl_repositories) const;

    void on_generate_state_not_relaxed_impl(State state,
                                            GroundAction action,
                                            ContinuousCost action_cost,
                                            Problem problem,
                                            const PDDLRepositories& pddl_repositories) const;

    void on_close_state_impl(State state, Problem problem, const PDDLRepositories& pddl_repositories) const;

    void on_finish_f_layer_impl(double f_value, uint64_t num_expanded_states, uint64_t num_generated_states) const;

    void on_prune_state_impl(State state, Problem problem, const PDDLRepositories& pddl_repositories) const;

    void on_start_search_impl(State start_state, Problem problem, const PDDLRepositories& pddl_repositories) const;

    void on_end_search_impl() const;

    void on_solved_impl(const Plan& plan, const PDDLRepositories& pddl_repositories) const;

    void on_unsolvable_impl() const;

    void on_exhausted_impl() const;

public:
    explicit DefaultAStarAlgorithmEventHandler(bool quiet = true) : StaticAStarAlgorithmEventHandlerBase<DefaultAStarAlgorithmEventHandler>(quiet) {}
};

}

#endif
