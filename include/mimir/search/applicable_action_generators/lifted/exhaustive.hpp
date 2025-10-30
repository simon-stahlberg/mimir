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

#ifndef MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_LIFTED_EXHAUSTIVE_HPP_
#define MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_LIFTED_EXHAUSTIVE_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/search/applicable_action_generators/interface.hpp"
#include "mimir/search/applicable_action_generators/lifted/exhaustive/event_handlers/statistics.hpp"
#include "mimir/search/declarations.hpp"

namespace mimir::search
{

/// @brief `ExhaustiveLiftedApplicableActionGeneratorImpl` implements lifted applicable action generation
/// that generates all possible bindings
class ExhaustiveLiftedApplicableActionGeneratorImpl : public IApplicableActionGenerator
{
public:
    using Statistics = applicable_action_generator::lifted::exhaustive::Statistics;

    using IEventHandler = applicable_action_generator::lifted::exhaustive::IEventHandler;
    using EventHandler = applicable_action_generator::lifted::exhaustive::EventHandler;

    using DebugEventHandlerImpl = applicable_action_generator::lifted::exhaustive::DebugEventHandlerImpl;
    using DebugEventHandler = applicable_action_generator::lifted::exhaustive::DebugEventHandler;

    using DefaultEventHandlerImpl = applicable_action_generator::lifted::exhaustive::DefaultEventHandlerImpl;
    using DefaultEventHandler = applicable_action_generator::lifted::exhaustive::DefaultEventHandler;

    ExhaustiveLiftedApplicableActionGeneratorImpl(formalism::Problem problem,
                                                  EventHandler event_handler = nullptr,
                                                  satisficing_binding_generator::EventHandler binding_event_handler = nullptr);

    static std::shared_ptr<ExhaustiveLiftedApplicableActionGeneratorImpl>
    create(formalism::Problem problem, EventHandler event_handler = nullptr, satisficing_binding_generator::EventHandler binding_event_handler = nullptr);

    // Uncopyable
    ExhaustiveLiftedApplicableActionGeneratorImpl(const ExhaustiveLiftedApplicableActionGeneratorImpl& other) = delete;
    ExhaustiveLiftedApplicableActionGeneratorImpl& operator=(const ExhaustiveLiftedApplicableActionGeneratorImpl& other) = delete;
    // Unmovable
    ExhaustiveLiftedApplicableActionGeneratorImpl(ExhaustiveLiftedApplicableActionGeneratorImpl&& other) = delete;
    ExhaustiveLiftedApplicableActionGeneratorImpl& operator=(ExhaustiveLiftedApplicableActionGeneratorImpl&& other) = delete;

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

    using ActionParameterBindings = std::vector<formalism::ObjectList>;

    std::vector<ActionParameterBindings> m_parameters_bindings_per_action;
};

}  // namespace mimir

#endif
