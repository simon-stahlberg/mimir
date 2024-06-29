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

#ifndef MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_GROUNDED_HPP_
#define MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_GROUNDED_HPP_

#include "mimir/formalism/formalism.hpp"
#include "mimir/search/applicable_action_generators/grounded/event_handlers.hpp"
#include "mimir/search/applicable_action_generators/grounded/match_tree.hpp"
#include "mimir/search/applicable_action_generators/interface.hpp"
#include "mimir/search/applicable_action_generators/lifted.hpp"

#include <variant>

namespace mimir
{

/**
 * Fully specialized implementation class.
 */
class GroundedApplicableActionGenerator : public IApplicableActionGenerator
{
private:
    Problem m_problem;

    // Memory
    std::shared_ptr<PDDLFactories> m_pddl_factories;
    std::shared_ptr<IGroundedAAGEventHandler> m_event_handler;

    LiftedAAG m_lifted_aag;

    MatchTree<GroundAction> m_action_match_tree;
    MatchTree<GroundAxiom> m_axiom_match_tree;

public:
    /// @brief Simplest construction
    GroundedApplicableActionGenerator(Problem problem, std::shared_ptr<PDDLFactories> pddl_factories);

    /// @brief Complete construction
    GroundedApplicableActionGenerator(Problem problem, std::shared_ptr<PDDLFactories> pddl_factories, std::shared_ptr<IGroundedAAGEventHandler> event_handler);

    // Uncopyable
    GroundedApplicableActionGenerator(const GroundedApplicableActionGenerator& other) = delete;
    GroundedApplicableActionGenerator& operator=(const GroundedApplicableActionGenerator& other) = delete;
    // Unmovable
    GroundedApplicableActionGenerator(GroundedApplicableActionGenerator&& other) = delete;
    GroundedApplicableActionGenerator& operator=(GroundedApplicableActionGenerator&& other) = delete;

    void generate_applicable_actions(State state, GroundActionList& out_applicable_actions) override;

    void generate_and_apply_axioms(const FlatBitsetBuilder<Fluent>& fluent_state_atoms, FlatBitsetBuilder<Derived>& ref_derived_state_atoms) override;

    void on_finish_f_layer() const override;

    void on_end_search() const override;

    [[nodiscard]] Problem get_problem() const override;

    [[nodiscard]] const std::shared_ptr<PDDLFactories>& get_pddl_factories() const override;

    /// @brief Return all actions.
    [[nodiscard]] const GroundActionList& get_ground_actions() const;

    /// @brief Return the action with the given id.
    [[nodiscard]] GroundAction get_action(size_t action_id) const override;
};

/**
 * Types
 */

using GroundedAAG = GroundedApplicableActionGenerator;

}

#endif