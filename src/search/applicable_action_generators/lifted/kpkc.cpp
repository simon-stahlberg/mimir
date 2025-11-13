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
#include "mimir/graphs/algorithms/color_refinement.hpp"
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
                                                                                 const SearchContextImpl::LiftedOptions::KPKCOptions& options,
                                                                                 EventHandler event_handler,
                                                                                 satisficing_binding_generator::EventHandler binding_event_handler) :
    m_problem(problem),
    m_options(options),
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

KPKCLiftedApplicableActionGenerator KPKCLiftedApplicableActionGeneratorImpl::create(Problem problem,
                                                                                    const SearchContextImpl::LiftedOptions::KPKCOptions& options,
                                                                                    EventHandler event_handler,
                                                                                    satisficing_binding_generator::EventHandler binding_event_handler)
{
    return std::make_shared<KPKCLiftedApplicableActionGeneratorImpl>(problem, options, event_handler, binding_event_handler);
}

mimir::generator<GroundAction> KPKCLiftedApplicableActionGeneratorImpl::create_applicable_action_generator(const State& state)
{
    initialize(state.get_unpacked_state(), m_dynamic_assignment_sets);

    /* Generate applicable actions */

    m_event_handler->on_start_generating_applicable_actions();

    const auto& ground_action_repository =
        boost::hana::at_key(state.get_problem().get_repositories().get_hana_repositories(), boost::hana::type<GroundActionImpl> {});

    if (m_options.pruning == SearchContextImpl::SymmetryPruning::OFF)
    {
        for (auto& condition_grounder : m_action_grounding_data)
        {
            // We move this check here to avoid unnecessary creations of mimir::generator.
            if (!nullary_conditions_hold(condition_grounder.get_conjunctive_condition(), state.get_unpacked_state()))
            {
                continue;
            }

            auto vertex_mask = std::optional<boost::dynamic_bitset<>> { std::nullopt };

            for (auto&& binding : condition_grounder.create_binding_generator(state, m_dynamic_assignment_sets, vertex_mask))
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
    }
    else
    {
        // --- Step 1: Create object graph, compute mapping from vertex to orbit where the object with index i corresponds to vertex with index i. ---

        auto object_graph = datasets::create_object_graph(state, *m_problem);
        // std::cout << object_graph << std::endl;

        auto vertex_to_orbit = IndexList(object_graph.get_num_vertices());

        if (m_options.pruning == SearchContextImpl::SymmetryPruning::GI)
        {
            auto nauty_graph = graphs::nauty::SparseGraph(object_graph);
            nauty_graph.canonize();
            // std::cout << "orbits: " << to_string(nauty_graph.get_orbits()) << std::endl;

            for (Index i = 0; i < (Index) object_graph.get_num_vertices(); ++i)
            {
                vertex_to_orbit[i] = nauty_graph.get_orbits()[i];
            }
        }
        else if (m_options.pruning == SearchContextImpl::SymmetryPruning::WL1)
        {
            const auto certificate = graphs::color_refinement::compute_certificate(object_graph);
            // std::cout << "orbits: " << to_string(certificate->get_hash_to_color()) << std::endl;

            auto color_to_index = IndexMap<Index> {};
            for (Index i = 0; i < (Index) object_graph.get_num_vertices(); ++i)
            {
                const auto [it, success] = color_to_index.emplace(certificate->get_hash_to_color()[i], color_to_index.size());
                vertex_to_orbit[i] = it->second;
            }
        }

        // std::cout << "vertex_to_orbit: " << to_string(vertex_to_orbit) << std::endl;

        for (auto& condition_grounder : m_action_grounding_data)
        {
            // We move this check here to avoid unnecessary creations of mimir::generator.
            if (!nullary_conditions_hold(condition_grounder.get_conjunctive_condition(), state.get_unpacked_state()))
            {
                continue;
            }

            // --- Step 2: Compute number of times each orbits is touched by an action parameter. ---

            auto touched_orbits = IndexSet {};
            auto count_touched_orbits = IndexList(object_graph.get_num_vertices(), 0);

            // std::cout << "get_objects_by_parameter_index: " << to_string(condition_grounder.get_static_consistency_graph().get_objects_by_parameter_index())
            //           << std::endl;

            for (const auto& objects : condition_grounder.get_static_consistency_graph().get_objects_by_parameter_index())
            {
                touched_orbits.clear();
                for (const auto& object : objects)
                {
                    touched_orbits.insert(vertex_to_orbit[object]);  // vertex index
                }
                for (const auto orbit : touched_orbits)
                {
                    ++count_touched_orbits[orbit];
                }
            }

            // std::cout << "action: " << condition_grounder.get_action()->get_name() << std::endl;
            // std::cout << "count_touched_orbits: " << to_string(count_touched_orbits) << std::endl;

            // --- Step 3: Compute a vertex mask where each vertex parameter that touches an orbit with count n sets the mask to 1 if the vertex object is among
            // the n lex smallest objects in the orbit. ---

            const auto num_objects = m_problem->get_problem_and_domain_objects().size();
            const auto arity = condition_grounder.get_action()->get_arity();
            auto reduced_objects = boost::dynamic_bitset<>(num_objects * arity);
            auto tmp_count_touched_orbits = IndexList {};

            for (size_t i = 0; i < condition_grounder.get_action()->get_arity(); ++i)
            {
                const auto& objects = condition_grounder.get_static_consistency_graph().get_objects_by_parameter_index()[i];

                tmp_count_touched_orbits = count_touched_orbits;

                for (const auto& object : objects)
                {
                    const auto orbit = vertex_to_orbit[object];

                    if (tmp_count_touched_orbits[orbit] > 0)
                    {
                        reduced_objects[i * num_objects + object] = true;
                        --tmp_count_touched_orbits[orbit];
                    }
                }
            }

            // std::cout << "objects_by_parameter_index: " << to_string(condition_grounder.get_static_consistency_graph().get_objects_by_parameter_index())
            //           << std::endl;
            // std::cout << "reduced_objects_by_parameter_index: " << to_string(reduced_objects_by_parameter_index) << std::endl << std::endl;

            auto vertex_mask =
                std::optional<boost::dynamic_bitset<>>(boost::dynamic_bitset<>(condition_grounder.get_static_consistency_graph().get_vertices().size(), false));

            for (const auto& vertex : condition_grounder.get_static_consistency_graph().get_vertices())
            {
                const auto parameter = vertex.get_parameter_index();
                const auto object = vertex.get_object_index();

                if (reduced_objects[parameter * num_objects + object])
                {
                    vertex_mask->set(vertex.get_index());
                }
            }

            for (auto&& binding : condition_grounder.create_binding_generator(state, m_dynamic_assignment_sets, vertex_mask))
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