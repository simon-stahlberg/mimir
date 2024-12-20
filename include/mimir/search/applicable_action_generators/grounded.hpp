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
#include "mimir/search/applicable_action_generators/interface.hpp"
#include "mimir/search/declarations.hpp"
#include "mimir/search/match_tree.hpp"

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
    std::shared_ptr<ActionGrounder> m_grounder;
    MatchTree<GroundAction> m_match_tree;

    std::shared_ptr<IGroundedApplicableActionGeneratorEventHandler> m_event_handler;

public:
    /// @brief Simplest construction
    GroundedApplicableActionGenerator(std::shared_ptr<ActionGrounder> action_grounder, MatchTree<GroundAction> match_tree);

    /// @brief Complete construction
    GroundedApplicableActionGenerator(std::shared_ptr<ActionGrounder> action_grounder,
                                      MatchTree<GroundAction> match_tree,
                                      std::shared_ptr<IGroundedApplicableActionGeneratorEventHandler> event_handler);

    // Uncopyable
    GroundedApplicableActionGenerator(const GroundedApplicableActionGenerator& other) = delete;
    GroundedApplicableActionGenerator& operator=(const GroundedApplicableActionGenerator& other) = delete;
    // Unmovable
    GroundedApplicableActionGenerator(GroundedApplicableActionGenerator&& other) = delete;
    GroundedApplicableActionGenerator& operator=(GroundedApplicableActionGenerator&& other) = delete;

    /// @brief Create a grounded applicable action generator for the given state.
    /// @param state is the state.
    /// @param workspace is the workspace. There is no specific workspace in the grounded case and it should not be initialized.
    /// @return a generator to yield the applicable actions for the given state.
    mimir::generator<GroundAction> create_applicable_action_generator(State state, ApplicableActionGeneratorWorkspace& workspace) override;
    mimir::generator<GroundAction> create_applicable_action_generator(const DenseState& dense_state, ApplicableActionGeneratorWorkspace& workspace) override;

    void on_finish_search_layer() override;
    void on_end_search() override;

    /**
     * Getters
     */

    Problem get_problem() const override;
    const std::shared_ptr<PDDLRepositories>& get_pddl_repositories() const override;
    const std::shared_ptr<ActionGrounder>& get_action_grounder() const override;
};

}

#endif