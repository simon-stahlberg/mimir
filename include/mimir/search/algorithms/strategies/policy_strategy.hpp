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

struct RolloutOptions
{
    size_t rollout_limit;
    bool stop_if_goal;
    bool stop_if_known_state;
    bool stop_if_cycle;
};

/// @brief `IRolloutGuide` encapsulates logic to perform policy execution during search.
class IRolloutGuide
{
public:
    /// @brief Create a `IRolloutGuide` with minimal amount of options.
    /// @param options
    explicit IRolloutGuide(const RolloutOptions options);

    virtual ~IRolloutGuide() = default;

    /// @brief Called when expanding a state.
    /// @param state is the state.
    virtual void on_expand(State state) = 0;

    /// @brief Called when generating a state.
    /// @param action is the action.
    /// @param successor_state is the successor state.
    /// @return true if the successor state must be part of the rollout. Can be used to enforce early termination of expansion step.
    virtual bool on_generate(State state, formalism::GroundAction action, State successor_state) = 0;

    /// @brief Called when finished expanding the state.
    /// @return the optionally chosen successor state that must be part of the rollout. If none is given, the rollout terminates.
    virtual std::optional<State> get_choice() = 0;

protected:
    RolloutOptions m_options;
};

/// @brief `RolloutExecutor` computes a rollout trajectory guided by a `RolloutGuide`.
/// @tparam ...SearchNodeProperties
template<typename... SearchNodeProperties>
class RolloutExecutor
{
public:
    explicit RolloutExecutor(RolloutGuide guide,
                             GoalStrategy goal_strategy,
                             SearchContext search_context,
                             const SearchNodeImplVector<SearchNodeProperties...>& search_nodes);

    const StateList& compute_rollout(State state);

private:
    RolloutGuide m_guide;
    GoalStrategy m_goal_strategy;
    SearchContext m_search_context;
    const SearchNodeImplVector<SearchNodeProperties...>& m_search_nodes;

    StateList m_rollout;
};

}

#endif