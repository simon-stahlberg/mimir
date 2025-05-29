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

#ifndef MIMIR_SEARCH_ALGORITHMS_STRATEGIES_POLICY_STRATEGY_HPP_
#define MIMIR_SEARCH_ALGORITHMS_STRATEGIES_POLICY_STRATEGY_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/search/declarations.hpp"
#include "mimir/search/search_node.hpp"

namespace mimir::search
{

/// @brief `IPolicyGuide` encapsulates logic to perform policy execution during search.
class IPolicyGuide
{
public:
    virtual ~IPolicyGuide() = default;

    /// @brief Called when expanding a state.
    /// @param state is the state.
    virtual void on_expand(State state) = 0;

    /// @brief Called when generating a state.
    /// Set return value to true to select early and finished expanding.
    /// @param action is the action.
    /// @param successor_state is the successor state.
    /// @return true if the successor state must be part of the rollout.
    virtual bool on_generate(formalism::GroundAction action, State successor_state) = 0;

    /// @brief Called when finished expanding the state.
    /// @return the chosen successor state that must be part of the rollout.
    virtual State get_choice() = 0;
};

struct PolicyOptions
{
    size_t rollout_limit;
    bool stop_if_goal;
    bool stop_if_known_state;
    bool stop_if_cycle;
};

template<typename... SearchNodeProperties>
class PolicyExecutor
{
public:
    explicit PolicyExecutor(PolicyGuide guide,
                            GoalStrategy goal_strategy,
                            const SearchNodeImplVector<SearchNodeProperties...>& search_nodes,
                            const SearchContext& context,
                            const PolicyOptions& options);

    void execute_lookahead(State state);

    const StateList& get_rollout() const;

private:
    const SearchNodeImplVector<SearchNodeProperties...>& m_search_nodes;

    StateList m_rollout;
};

}

#endif