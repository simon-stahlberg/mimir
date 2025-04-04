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
#include "mimir/formalism/ground_action.hpp"
#include "mimir/search/applicable_action_generators/interface.hpp"
#include "mimir/search/declarations.hpp"
#include "mimir/search/dense_state.hpp"
#include "mimir/search/match_tree/declarations.hpp"
#include "mimir/search/match_tree/match_tree.hpp"

#include <variant>

namespace mimir::search
{

/// @brief `GroundedApplicableActionGenerator` implements grounded applicable action generation
/// using the `LiftedApplicableActionGenerator` and `AxiomEvaluator` to create an overapproximation
/// of applicable ground actions and ground actions and storing them in a match tree
/// as described by Helmert
class GroundedApplicableActionGeneratorImpl : public IApplicableActionGenerator
{
public:
    using Statistics = applicable_action_generator::grounded::Statistics;

    using IEventHandler = applicable_action_generator::grounded::IEventHandler;
    using EventHandler = applicable_action_generator::grounded::EventHandler;

    using DebugEventHandlerImpl = applicable_action_generator::grounded::DebugEventHandlerImpl;
    using DebugEventHandler = applicable_action_generator::grounded::DebugEventHandler;

    using DefaultEventHandlerImpl = applicable_action_generator::grounded::DefaultEventHandlerImpl;
    using DefaultEventHandler = applicable_action_generator::grounded::DefaultEventHandler;

    /// @brief Complete construction
    GroundedApplicableActionGeneratorImpl(formalism::Problem problem,
                                          match_tree::MatchTree<formalism::GroundActionImpl>&& match_tree,
                                          EventHandler event_handler);

    /// @brief Simplest construction
    static GroundedApplicableActionGenerator create(formalism::Problem problem, match_tree::MatchTree<formalism::GroundActionImpl>&& match_tree);

    /// @brief Complete construction
    static GroundedApplicableActionGenerator
    create(formalism::Problem problem, match_tree::MatchTree<formalism::GroundActionImpl>&& match_tree, EventHandler event_handler);

    // Uncopyable
    GroundedApplicableActionGeneratorImpl(const GroundedApplicableActionGeneratorImpl& other) = delete;
    GroundedApplicableActionGeneratorImpl& operator=(const GroundedApplicableActionGeneratorImpl& other) = delete;
    // Unmovable
    GroundedApplicableActionGeneratorImpl(GroundedApplicableActionGeneratorImpl&& other) = delete;
    GroundedApplicableActionGeneratorImpl& operator=(GroundedApplicableActionGeneratorImpl&& other) = delete;

    /// @brief Create a grounded applicable action generator for the given state.
    /// @param state is the state.
    /// @param workspace is the workspace. There is no specific workspace in the grounded case and it should not be initialized.
    /// @return a generator to yield the applicable actions for the given state.
    mimir::generator<formalism::GroundAction> create_applicable_action_generator(State state) override;
    mimir::generator<formalism::GroundAction> create_applicable_action_generator(const DenseState& dense_state) override;

    void on_finish_search_layer() override;
    void on_end_search() override;

    /**
     * Getters
     */

    const formalism::Problem& get_problem() const override;

private:
    formalism::Problem m_problem;
    match_tree::MatchTree<formalism::GroundActionImpl> m_match_tree;

    EventHandler m_event_handler;

    /* Memory for reuse */
    DenseState m_dense_state;
};

}

#endif