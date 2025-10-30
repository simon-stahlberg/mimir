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

#include "mimir/search/applicable_action_generators/lifted/exhaustive.hpp"

#include "mimir/common/formatter.hpp"
#include "mimir/common/itertools.hpp"
#include "mimir/formalism/consistency_graph.hpp"
#include "mimir/formalism/domain.hpp"
#include "mimir/formalism/ground_action.hpp"
#include "mimir/formalism/object.hpp"
#include "mimir/formalism/problem.hpp"
#include "mimir/formalism/repositories.hpp"
#include "mimir/search/applicability.hpp"
#include "mimir/search/applicable_action_generators/lifted/exhaustive/event_handlers/default.hpp"
#include "mimir/search/applicable_action_generators/lifted/exhaustive/event_handlers/interface.hpp"
#include "mimir/search/satisficing_binding_generators/event_handlers/default.hpp"
#include "mimir/search/state.hpp"

#include <stdexcept>
#include <vector>

using namespace mimir::formalism;

using namespace std::string_literals;

namespace mimir::search
{

/**
 * LiftedApplicableActionGenerator
 */

ExhaustiveLiftedApplicableActionGeneratorImpl::ExhaustiveLiftedApplicableActionGeneratorImpl(
    Problem problem,
    EventHandler event_handler,
    satisficing_binding_generator::EventHandler binding_event_handler) :
    m_problem(problem),
    m_event_handler(event_handler ? event_handler : DefaultEventHandlerImpl::create()),
    m_binding_event_handler(binding_event_handler ? binding_event_handler : satisficing_binding_generator::DefaultEventHandlerImpl::create()),
    m_parameters_bindings_per_action()
{
    for (const auto action : m_problem->get_domain()->get_actions())
    {
        assert(action->get_index() == m_parameters_bindings_per_action.size());

        auto objects_by_parameter_index =
            std::get<2>(StaticConsistencyGraph::compute_vertices(*m_problem, action->get_conjunctive_condition(), 0, action->get_arity()));

        auto parameters_bindings = ActionParameterBindings {};
        parameters_bindings.reserve(objects_by_parameter_index.size());
        for (const auto& idxs : objects_by_parameter_index)
        {
            parameters_bindings.push_back(m_problem->get_repositories().get_objects_from_indices(idxs));
        }

        m_parameters_bindings_per_action.push_back(std::move(parameters_bindings));
    }
}

std::shared_ptr<ExhaustiveLiftedApplicableActionGeneratorImpl>
ExhaustiveLiftedApplicableActionGeneratorImpl::create(Problem problem,
                                                      EventHandler event_handler,
                                                      satisficing_binding_generator::EventHandler binding_event_handler)
{
    return std::shared_ptr<ExhaustiveLiftedApplicableActionGeneratorImpl>(
        new ExhaustiveLiftedApplicableActionGeneratorImpl(problem, event_handler, binding_event_handler));
}

mimir::generator<GroundAction> ExhaustiveLiftedApplicableActionGeneratorImpl::create_applicable_action_generator(const State& state)
{
    m_event_handler->on_start_generating_applicable_actions();

    const auto& ground_action_repository = boost::hana::at_key(m_problem->get_repositories().get_hana_repositories(), boost::hana::type<GroundActionImpl> {});

    for (size_t i = 0; i < m_problem->get_domain()->get_actions().size(); ++i)
    {
        const auto action = m_problem->get_domain()->get_actions()[i];
        const auto& parameters_bindings = m_parameters_bindings_per_action[i];

        for (const auto& binding : create_cartesian_product_generator(parameters_bindings))
        {
            const auto num_ground_actions = ground_action_repository.size();

            const auto ground_action = m_problem->ground(action, binding);

            if (is_applicable(ground_action, state))
            {
                // TODO(Dominik): This is awkward. Should have separate EventHandler
                m_binding_event_handler->on_valid_base_binding(binding);
                m_binding_event_handler->on_valid_derived_binding(binding);

                m_event_handler->on_ground_action(ground_action);

                (ground_action_repository.size() > num_ground_actions) ? m_event_handler->on_ground_action_cache_miss(ground_action) :
                                                                         m_event_handler->on_ground_action_cache_hit(ground_action);

                co_yield ground_action;
            }
            else
            {
                m_binding_event_handler->on_invalid_base_binding(binding);
                m_binding_event_handler->on_invalid_derived_binding(binding);
            }
        }
    }

    m_event_handler->on_end_generating_applicable_actions();
}

const Problem& ExhaustiveLiftedApplicableActionGeneratorImpl::get_problem() const { return m_problem; }

void ExhaustiveLiftedApplicableActionGeneratorImpl::on_finish_search_layer()
{
    m_event_handler->on_finish_search_layer();
    m_binding_event_handler->on_finish_search_layer();
}

void ExhaustiveLiftedApplicableActionGeneratorImpl::on_end_search()
{
    m_event_handler->on_end_search();
    m_binding_event_handler->on_end_search();
}
}