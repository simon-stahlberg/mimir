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
#include "mimir/search/applicable_action_generators/lifted/kpkc/event_handlers/statistics.hpp"
#include "mimir/search/declarations.hpp"
#include "mimir/search/satisficing_binding_generators/action.hpp"

namespace mimir::search
{

/// @brief `KPKCLiftedApplicableActionGeneratorImpl` implements lifted applicable action generation
/// using maximum clique enumeration by Stahlberg (ECAI2023).
/// Source: https://mrlab.ai/papers/stahlberg-ecai2023.pdf
class KPKCLiftedApplicableActionGeneratorImpl : public IApplicableActionGenerator
{
public:
    using Statistics = applicable_action_generator::lifted::kpkc::Statistics;

    using IEventHandler = applicable_action_generator::lifted::kpkc::IEventHandler;
    using EventHandler = applicable_action_generator::lifted::kpkc::EventHandler;

    using DebugEventHandlerImpl = applicable_action_generator::lifted::kpkc::DebugEventHandlerImpl;
    using DebugEventHandler = applicable_action_generator::lifted::kpkc::DebugEventHandler;

    using DefaultEventHandlerImpl = applicable_action_generator::lifted::kpkc::DefaultEventHandlerImpl;
    using DefaultEventHandler = applicable_action_generator::lifted::kpkc::DefaultEventHandler;

    KPKCLiftedApplicableActionGeneratorImpl(formalism::Problem problem,
                                            EventHandler event_handler = nullptr,
                                            satisficing_binding_generator::EventHandler binding_event_handler = nullptr);

    static KPKCLiftedApplicableActionGenerator
    create(formalism::Problem problem, EventHandler event_handler = nullptr, satisficing_binding_generator::EventHandler binding_event_handler = nullptr);

    // Uncopyable
    KPKCLiftedApplicableActionGeneratorImpl(const KPKCLiftedApplicableActionGeneratorImpl& other) = delete;
    KPKCLiftedApplicableActionGeneratorImpl& operator=(const KPKCLiftedApplicableActionGeneratorImpl& other) = delete;
    // Unmovable
    KPKCLiftedApplicableActionGeneratorImpl(KPKCLiftedApplicableActionGeneratorImpl&& other) = delete;
    KPKCLiftedApplicableActionGeneratorImpl& operator=(KPKCLiftedApplicableActionGeneratorImpl&& other) = delete;

    mimir::generator<formalism::GroundAction> create_applicable_action_generator(const State& state) override;

    void on_finish_search_layer() override;
    void on_end_search() override;

    /**
     * Getters
     */

    const formalism::Problem& get_problem() const override;

private:
    formalism::Problem m_problem;
    EventHandler m_event_handler;
    satisficing_binding_generator::EventHandler m_binding_event_handler;

    ActionSatisficingBindingGeneratorList m_action_grounding_data;

    /* Memory for reuse */
    formalism::GroundAtomList<formalism::FluentTag> m_fluent_atoms;
    formalism::GroundAtomList<formalism::DerivedTag> m_derived_atoms;
    formalism::GroundFunctionList<formalism::FluentTag> m_fluent_functions;
    formalism::PredicateAssignmentSets<formalism::FluentTag> m_fluent_predicate_assignment_sets;
    formalism::PredicateAssignmentSets<formalism::DerivedTag> m_derived_predicate_assignment_sets;
    formalism::FunctionSkeletonAssignmentSets<formalism::FluentTag> m_fluent_function_skeleton_assignment_sets;
};

}  // namespace mimir

#endif
