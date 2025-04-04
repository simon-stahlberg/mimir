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

#ifndef MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_LIFTED_HPP_
#define MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_LIFTED_HPP_

#include "mimir/formalism/assignment_set.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/search/applicable_action_generators/interface.hpp"
#include "mimir/search/applicable_action_generators/lifted/event_handlers/statistics.hpp"
#include "mimir/search/declarations.hpp"
#include "mimir/search/satisficing_binding_generators/action.hpp"

namespace mimir::search
{

/// @brief `LiftedApplicableActionGeneratorImpl` implements lifted applicable action generation
/// using maximum clique enumeration by Stahlberg (ECAI2023).
/// Source: https://mrlab.ai/papers/stahlberg-ecai2023.pdf
class LiftedApplicableActionGeneratorImpl : public IApplicableActionGenerator
{
public:
    using Statistics = applicable_action_generator::lifted::Statistics;

    using IEventHandler = applicable_action_generator::lifted::IEventHandler;
    using EventHandler = applicable_action_generator::lifted::EventHandler;

    using DebugEventHandlerImpl = applicable_action_generator::lifted::DebugEventHandlerImpl;
    using DebugEventHandler = applicable_action_generator::lifted::DebugEventHandler;

    using DefaultEventHandlerImpl = applicable_action_generator::lifted::DefaultEventHandlerImpl;
    using DefaultEventHandler = applicable_action_generator::lifted::DefaultEventHandler;

    /// @brief Simplest construction
    LiftedApplicableActionGeneratorImpl(formalism::Problem problem);

    /// @brief Complete construction
    LiftedApplicableActionGeneratorImpl(formalism::Problem problem, EventHandler event_handler);

    /// @brief Simplest construction
    static std::shared_ptr<LiftedApplicableActionGeneratorImpl> create(formalism::Problem problem);

    /// @brief Complete construction
    static std::shared_ptr<LiftedApplicableActionGeneratorImpl> create(formalism::Problem problem, EventHandler event_handler);

    // Uncopyable
    LiftedApplicableActionGeneratorImpl(const LiftedApplicableActionGeneratorImpl& other) = delete;
    LiftedApplicableActionGeneratorImpl& operator=(const LiftedApplicableActionGeneratorImpl& other) = delete;
    // Unmovable
    LiftedApplicableActionGeneratorImpl(LiftedApplicableActionGeneratorImpl&& other) = delete;
    LiftedApplicableActionGeneratorImpl& operator=(LiftedApplicableActionGeneratorImpl&& other) = delete;

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
    EventHandler m_event_handler;

    ActionSatisficingBindingGeneratorList m_action_grounding_data;

    /* Memory for reuse */
    DenseState m_dense_state;
    formalism::GroundAtomList<formalism::FluentTag> m_fluent_atoms;
    formalism::GroundAtomList<formalism::DerivedTag> m_derived_atoms;
    formalism::GroundFunctionList<formalism::FluentTag> m_fluent_functions;
    formalism::AssignmentSet<formalism::FluentTag> m_fluent_assignment_set;
    formalism::AssignmentSet<formalism::DerivedTag> m_derived_assignment_set;
    formalism::NumericAssignmentSet<formalism::FluentTag> m_numeric_assignment_set;
};

}  // namespace mimir

#endif
