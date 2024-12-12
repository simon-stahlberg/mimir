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

#ifndef MIMIR_SEARCH_GROUNDERS_ACTION_GROUNDER_HPP_
#define MIMIR_SEARCH_GROUNDERS_ACTION_GROUNDER_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/search/action.hpp"
#include "mimir/search/consistency_graph.hpp"
#include "mimir/search/declarations.hpp"
#include "mimir/search/grounders/function_grounder.hpp"
#include "mimir/search/grounders/grounding_table.hpp"
#include "mimir/search/grounders/literal_grounder.hpp"
#include "mimir/search/satisficing_binding_generator.hpp"

#include <unordered_map>
#include <vector>

namespace mimir
{
class ActionGrounder
{
private:
    std::shared_ptr<LiteralGrounder> m_literal_grounder;
    std::shared_ptr<FunctionGrounder> m_function_grounder;

    GroundActionImplSet m_actions;
    GroundActionList m_actions_by_index;
    GroundActionImpl m_action_builder;
    std::unordered_map<Action, GroundingTable<GroundAction>> m_action_groundings;

    // TODO: we can think about moving the consistency graph into the action schemas
    // TODO: actually we could also only store the part of the consistency graph that we really need here.
    std::unordered_map<Action, std::vector<consistency_graph::StaticConsistencyGraph>> m_action_conditional_effects;

public:
    /// @brief Simplest construction
    ActionGrounder(std::shared_ptr<LiteralGrounder> literal_grounder, std::shared_ptr<FunctionGrounder> function_grounder);

    // Uncopyable
    ActionGrounder(const ActionGrounder& other) = delete;
    ActionGrounder& operator=(const ActionGrounder& other) = delete;
    // Moveable
    ActionGrounder(ActionGrounder&& other) = default;
    ActionGrounder& operator=(ActionGrounder&& other) = default;

    /// @brief Ground an action and return a view onto it.
    GroundAction ground_action(Action action, ObjectList binding);

    /**
     * Getters
     */

    Problem get_problem() const;
    const std::shared_ptr<PDDLRepositories>& get_pddl_repositories() const;
    const std::shared_ptr<LiteralGrounder>& get_literal_grounder() const;
    const std::shared_ptr<FunctionGrounder>& get_function_grounder() const;

    const GroundActionList& get_ground_actions() const;
    GroundAction get_ground_action(Index action_index) const;
    size_t get_num_ground_actions() const;
};

}  // namespace mimir

#endif
