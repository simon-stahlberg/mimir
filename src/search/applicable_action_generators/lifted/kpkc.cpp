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

#include "mimir/search/applicable_action_generators/lifted/kpkc.hpp"

#include "mimir/common/formatter.hpp"
#include "mimir/datasets/object_graph.hpp"
#include "mimir/formalism/domain.hpp"
#include "mimir/formalism/formatter.hpp"
#include "mimir/formalism/ground_action.hpp"
#include "mimir/formalism/object.hpp"
#include "mimir/formalism/problem.hpp"
#include "mimir/formalism/repositories.hpp"
#include "mimir/graphs/algorithms/nauty.hpp"
#include "mimir/graphs/formatter.hpp"
#include "mimir/search/applicability.hpp"
#include "mimir/search/applicable_action_generators/lifted/kpkc/event_handlers/default.hpp"
#include "mimir/search/applicable_action_generators/lifted/kpkc/event_handlers/interface.hpp"
#include "mimir/search/assignment_set_utils.hpp"
#include "mimir/search/satisficing_binding_generators/event_handlers/default.hpp"
#include "mimir/search/satisficing_binding_generators/event_handlers/interface.hpp"
#include "mimir/search/state.hpp"

#include <boost/dynamic_bitset.hpp>
#include <stdexcept>
#include <vector>

using namespace mimir::formalism;

using namespace std::string_literals;

namespace mimir::search
{

/**
 * LiftedApplicableActionGenerator
 */

KPKCLiftedApplicableActionGeneratorImpl::KPKCLiftedApplicableActionGeneratorImpl(Problem problem,
                                                                                 EventHandler event_handler,
                                                                                 satisficing_binding_generator::EventHandler binding_event_handler) :
    m_problem(problem),
    m_event_handler(event_handler ? event_handler : DefaultEventHandlerImpl::create()),
    m_binding_event_handler(binding_event_handler ? binding_event_handler : satisficing_binding_generator::DefaultEventHandlerImpl::create()),
    m_action_grounding_data(),
    m_dynamic_assignment_sets(*m_problem)
{
    /* 2. Initialize the condition grounders for each action schema. */
    const auto& actions = problem->get_domain()->get_actions();
    for (size_t i = 0; i < actions.size(); ++i)
    {
        const auto& action = actions[i];
        assert(action->get_index() == i);
        m_action_grounding_data.push_back(ActionSatisficingBindingGenerator(action, m_problem, m_binding_event_handler));
    }
}

KPKCLiftedApplicableActionGenerator
KPKCLiftedApplicableActionGeneratorImpl::create(Problem problem, EventHandler event_handler, satisficing_binding_generator::EventHandler binding_event_handler)
{
    return std::make_shared<KPKCLiftedApplicableActionGeneratorImpl>(problem, event_handler, binding_event_handler);
}

mimir::generator<GroundAction> KPKCLiftedApplicableActionGeneratorImpl::create_applicable_action_generator(const State& state)
{
    initialize(state.get_unpacked_state(), m_dynamic_assignment_sets);

    /* Generate applicable actions */

    m_event_handler->on_start_generating_applicable_actions();

    const auto& ground_action_repository =
        boost::hana::at_key(state.get_problem().get_repositories().get_hana_repositories(), boost::hana::type<GroundActionImpl> {});

    auto [object_graph, object_to_vertex] = datasets::create_object_graph(state, *m_problem);
    std::cout << object_graph << std::endl;
    std::cout << "object_to_vertex: " << to_string(object_to_vertex) << std::endl;
    auto nauty_graph = graphs::nauty::SparseGraph(object_graph);
    nauty_graph.canonize();
    std::cout << "lab: " << to_string(nauty_graph.get_lab()) << std::endl;
    std::cout << "ptn: " << to_string(nauty_graph.get_ptn()) << std::endl;

    auto vertex_to_orbit = IndexList(nauty_graph.get_nv());
    auto orbit_to_vertices = std::vector<IndexList> {};
    auto vertices = IndexList {};
    for (Index i = 0; i < (Index) nauty_graph.get_nv(); ++i)
    {
        vertices.push_back(nauty_graph.get_lab()[i]);
        vertex_to_orbit[nauty_graph.get_lab()[i]] = orbit_to_vertices.size();

        if (nauty_graph.get_ptn()[i] == 0)  // finish orbit
        {
            std::sort(vertices.begin(), vertices.end());
            orbit_to_vertices.push_back(vertices);
            vertices.clear();
        }
    }

    std::cout << "vertex_to_orbit: " << to_string(vertex_to_orbit) << std::endl;
    std::cout << "orbit_to_vertices: " << to_string(orbit_to_vertices) << std::endl << std::endl;

    for (auto& condition_grounder : m_action_grounding_data)
    {
        // We move this check here to avoid unnecessary creations of mimir::generator.
        if (!nullary_conditions_hold(condition_grounder.get_conjunctive_condition(), state.get_unpacked_state()))
        {
            continue;
        }

        auto touched_orbits_per_parameter = std::vector<IndexSet> {};
        auto touched_orbits = IndexSet {};
        auto count_touched_orbits = std::vector<Index>(nauty_graph.get_nv(), 0);

        for (const auto& objects : condition_grounder.get_static_consistency_graph().get_objects_by_parameter_index())
        {
            for (const auto& object : objects)
            {
                touched_orbits.insert(vertex_to_orbit[object_to_vertex[object]]);  // vertex index
            }
            for (const auto orbit : touched_orbits)
            {
                ++count_touched_orbits[orbit];
            }
            touched_orbits_per_parameter.push_back(touched_orbits);
            touched_orbits.clear();
        }

        std::cout << "touched_orbits_per_parameter: " << to_string(touched_orbits_per_parameter) << std::endl;
        std::cout << "count_touched_orbits: " << to_string(count_touched_orbits) << std::endl << std::endl;

        for (auto&& binding : condition_grounder.create_binding_generator(state, m_dynamic_assignment_sets))
        {
            const auto num_ground_actions = ground_action_repository.size();

            const auto ground_action = m_problem->ground(condition_grounder.get_action(), std::move(binding));

            assert(is_applicable(ground_action, state));

            m_event_handler->on_ground_action(ground_action);

            (ground_action_repository.size() > num_ground_actions) ? m_event_handler->on_ground_action_cache_miss(ground_action) :
                                                                     m_event_handler->on_ground_action_cache_hit(ground_action);

            co_yield ground_action;
        }
    }

    m_event_handler->on_end_generating_applicable_actions();
}

const Problem& KPKCLiftedApplicableActionGeneratorImpl::get_problem() const { return m_problem; }

void KPKCLiftedApplicableActionGeneratorImpl::on_finish_search_layer()
{
    m_event_handler->on_finish_search_layer();
    m_binding_event_handler->on_finish_search_layer();
}

void KPKCLiftedApplicableActionGeneratorImpl::on_end_search()
{
    m_event_handler->on_end_search();
    m_binding_event_handler->on_end_search();
}
}