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

#ifndef MIMIR_SEARCH_GROUNDING_ACTION_GROUNDER_HPP_
#define MIMIR_SEARCH_GROUNDING_ACTION_GROUNDER_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/formalism/grounding_table.hpp"
#include "mimir/search/action.hpp"
#include "mimir/search/declarations.hpp"
#include "mimir/search/grounding/condition_grounder.hpp"
#include "mimir/search/grounding/consistency_graph.hpp"

#include <unordered_map>
#include <vector>

namespace mimir
{
using GroundFunctionToValue = std::unordered_map<GroundFunction, double>;

class ActionGrounder
{
private:
    Problem m_problem;
    std::shared_ptr<PDDLRepositories> m_pddl_repositories;

    std::unordered_map<Action, ConditionGrounder> m_action_precondition_grounders;
    std::unordered_map<Action, std::vector<consistency_graph::StaticConsistencyGraph>> m_action_conditional_effects;

    GroundActionImplSet m_actions;
    GroundActionList m_actions_by_index;
    GroundActionImpl m_action_builder;
    std::unordered_map<Action, GroundingTable<GroundAction>> m_action_groundings;

    GroundFunctionToValue m_ground_function_to_cost;

public:
    /// @brief Simplest construction
    ActionGrounder(Problem problem, std::shared_ptr<PDDLRepositories> pddl_repositories);

    // Uncopyable
    ActionGrounder(const ActionGrounder& other) = delete;
    ActionGrounder& operator=(const ActionGrounder& other) = delete;
    // Moveable
    ActionGrounder(ActionGrounder&& other) = default;
    ActionGrounder& operator=(ActionGrounder&& other) = default;

    Problem get_problem() const;

    const std::shared_ptr<PDDLRepositories>& get_pddl_repositories() const;

    std::unordered_map<Action, ConditionGrounder>& get_action_precondition_grounders();

    /// @brief Ground an action and return a view onto it.
    GroundAction ground_action(Action action, ObjectList binding);

    const GroundActionList& get_ground_actions() const;

    GroundAction get_ground_action(Index action_index) const;

    size_t get_num_ground_actions() const;
};

}  // namespace mimir

#endif
