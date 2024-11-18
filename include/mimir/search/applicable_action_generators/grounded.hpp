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

#include "mimir/formalism/declarations.hpp"
#include "mimir/search/applicable_action_generators/grounded/event_handlers.hpp"
#include "mimir/search/applicable_action_generators/grounded/match_tree.hpp"
#include "mimir/search/applicable_action_generators/interface.hpp"
#include "mimir/search/applicable_action_generators/lifted.hpp"
#include "mimir/search/declarations.hpp"

#include <variant>

namespace mimir
{

/// @brief `GroundedApplicableActionGenerator` implements grounded applicable action generation
/// using the `LiftedApplicableActionGenerator` and `AxiomEvaluator` to create an overapproximation
/// of applicable ground actions and ground actions and storing them in a match tree
/// as described by Helmert
class GroundedApplicableActionGenerator : public IApplicableActionGenerator
{
private:
    Problem m_problem;

    // Memory
    std::shared_ptr<PDDLRepositories> m_pddl_repositories;
    std::shared_ptr<IGroundedApplicableActionGeneratorEventHandler> m_event_handler;
    LiftedApplicableActionGenerator m_lifted_applicable_action_generator;

    MatchTree<GroundAction> m_action_match_tree;
    MatchTree<GroundAxiom> m_axiom_match_tree;

public:
    /// @brief Simplest construction
    GroundedApplicableActionGenerator(Problem problem, std::shared_ptr<PDDLRepositories> pddl_repositories);

    /// @brief Complete construction
    GroundedApplicableActionGenerator(Problem problem,
                                      std::shared_ptr<PDDLRepositories> pddl_repositories,
                                      std::shared_ptr<IGroundedApplicableActionGeneratorEventHandler> event_handler);

    // Uncopyable
    GroundedApplicableActionGenerator(const GroundedApplicableActionGenerator& other) = delete;
    GroundedApplicableActionGenerator& operator=(const GroundedApplicableActionGenerator& other) = delete;
    // Unmovable
    GroundedApplicableActionGenerator(GroundedApplicableActionGenerator&& other) = delete;
    GroundedApplicableActionGenerator& operator=(GroundedApplicableActionGenerator&& other) = delete;

    void generate_applicable_actions(State state, GroundActionList& out_applicable_actions) override;

    void generate_and_apply_axioms(StateImpl& unextended_state) override;

    void on_finish_search_layer() const override;

    void on_end_search() const override;

    Problem get_problem() const override;

    const std::shared_ptr<PDDLRepositories>& get_pddl_repositories() const override;

    const GroundActionList& get_ground_actions() const override;

    GroundAction get_ground_action(Index action_index) const override;

    const GroundAxiomList& get_ground_axioms() const override;

    GroundAxiom get_ground_axiom(Index axiom_index) const override;

    size_t get_num_ground_actions() const override;

    size_t get_num_ground_axioms() const override;
};

}

#endif