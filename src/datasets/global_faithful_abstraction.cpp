/*
 * Copyright (C) 2023 Simon Stahlberg
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

#include "mimir/datasets/global_faithful_abstraction.hpp"

#include "mimir/algorithms/BS_thread_pool.hpp"
#include "mimir/common/equal_to.hpp"
#include "mimir/common/hash.hpp"
#include "mimir/common/timers.hpp"

#include <algorithm>
#include <cstdlib>
#include <deque>

size_t std::hash<mimir::GlobalFaithfulAbstractState>::operator()(const mimir::GlobalFaithfulAbstractState& element) const
{
    return mimir::hash_combine(element.get_index(),
                               element.get_global_index(),
                               element.get_faithful_abstraction_index(),
                               element.get_faithful_abstract_state_index());
}

namespace mimir
{
/**
 * GlobalFaithfulAbstractState
 */

GlobalFaithfulAbstractState::GlobalFaithfulAbstractState(StateIndex index,
                                                         StateIndex global_index,
                                                         AbstractionIndex faithful_abstraction_index,
                                                         StateIndex faithful_abstract_state_index) :
    m_index(index),
    m_global_index(global_index),
    m_faithful_abstraction_index(faithful_abstraction_index),
    m_faithful_abstract_state_index(faithful_abstract_state_index)
{
}

bool GlobalFaithfulAbstractState::operator==(const GlobalFaithfulAbstractState& other) const
{
    if (this != &other)
    {
        return (m_index == other.m_index) && (m_global_index == other.m_global_index) && (m_faithful_abstraction_index == other.m_faithful_abstraction_index)
               && (m_faithful_abstract_state_index == other.m_faithful_abstract_state_index);
    }
    return true;
}

StateIndex GlobalFaithfulAbstractState::get_index() const { return m_index; }

StateIndex GlobalFaithfulAbstractState::get_global_index() const { return m_global_index; }

AbstractionIndex GlobalFaithfulAbstractState::get_faithful_abstraction_index() const { return m_faithful_abstraction_index; }

StateIndex GlobalFaithfulAbstractState::get_faithful_abstract_state_index() const { return m_faithful_abstract_state_index; }

/**
 * GlobalFaithfulAbstraction
 */

GlobalFaithfulAbstraction::GlobalFaithfulAbstraction(bool mark_true_goal_literals,
                                                     bool use_unit_cost_one,
                                                     AbstractionIndex id,
                                                     std::shared_ptr<const FaithfulAbstractionList> abstractions,
                                                     GlobalFaithfulAbstractStateList states,
                                                     size_t num_isomorphic_states,
                                                     size_t num_non_isomorphic_states) :
    m_mark_true_goal_literals(mark_true_goal_literals),
    m_use_unit_cost_one(use_unit_cost_one),
    m_index(id),
    m_abstractions(std::move(abstractions)),
    m_states(std::move(states)),
    m_num_isomorphic_states(num_isomorphic_states),
    m_num_non_isomorphic_states(num_non_isomorphic_states),
    m_global_state_index_to_state_index()
{
    /* Ensure correctness. */

    // Check correct state ordering
    for (size_t i = 0; i < get_num_states(); ++i)
    {
        assert(get_states().at(i).get_index() == static_cast<StateIndex>(i) && "State index does not match its position in the list");
    }

    /* Additional */
    for (const auto& state : m_states)
    {
        m_global_state_index_to_state_index.emplace(state.get_global_index(), state.get_index());
    }
}

std::vector<GlobalFaithfulAbstraction>
GlobalFaithfulAbstraction::create(const fs::path& domain_filepath, const std::vector<fs::path>& problem_filepaths, const FaithfulAbstractionsOptions& options)
{
    auto memories =
        std::vector<std::tuple<Problem, std::shared_ptr<PDDLFactories>, std::shared_ptr<IApplicableActionGenerator>, std::shared_ptr<StateRepository>>> {};
    for (const auto& problem_filepath : problem_filepaths)
    {
        auto parser = PDDLParser(domain_filepath, problem_filepath);
        auto aag = std::make_shared<GroundedApplicableActionGenerator>(parser.get_problem(), parser.get_pddl_factories());
        auto ssg = std::make_shared<StateRepository>(aag);
        memories.emplace_back(parser.get_problem(), parser.get_pddl_factories(), aag, ssg);
    }

    return GlobalFaithfulAbstraction::create(memories, options);
}

std::vector<GlobalFaithfulAbstraction> GlobalFaithfulAbstraction::create(
    const std::vector<std::tuple<Problem, std::shared_ptr<PDDLFactories>, std::shared_ptr<IApplicableActionGenerator>, std::shared_ptr<StateRepository>>>&
        memories,
    const FaithfulAbstractionsOptions& options)
{
    auto abstractions = std::vector<GlobalFaithfulAbstraction> {};
    auto faithful_abstractions = FaithfulAbstraction::create(memories, options);

    auto certificate_to_global_state = std::
        unordered_map<std::shared_ptr<const Certificate>, GlobalFaithfulAbstractState, UniqueCertificateSharedPtrHash, UniqueCertificateSharedPtrEqualTo> {};

    // An abstraction is considered relevant, if it contains at least one non-isomorphic state.
    auto relevant_faithful_abstractions = std::make_shared<FaithfulAbstractionList>();
    auto abstraction_index = AbstractionIndex { 0 };

    for (auto& faithful_abstraction : faithful_abstractions)
    {
        auto has_zero_non_isomorphic_states = certificate_to_global_state.count(
            mimir::get_certificate(faithful_abstraction.get_graph().get_vertices().at(faithful_abstraction.get_initial_state())));

        if (has_zero_non_isomorphic_states)
        {
            // Skip: no non-isomorphic state
            continue;
        }

        auto num_isomorphic_states = 0;
        auto num_non_isomorphic_states = 0;
        auto states = GlobalFaithfulAbstractStateList {};
        for (const auto& state : faithful_abstraction.get_graph().get_vertices())
        {
            // Ensure ordering consistent with state in faithful abstraction.
            assert(state.get_index() == states.size());

            auto it = certificate_to_global_state.find(mimir::get_certificate(state));

            if (it != certificate_to_global_state.end())
            {
                // Copy existing global state data.
                states.emplace_back(state.get_index(),
                                    it->second.get_global_index(),
                                    it->second.get_faithful_abstraction_index(),
                                    it->second.get_faithful_abstract_state_index());
                ++num_isomorphic_states;

                // Ensure that goals remain goals and deadends remain deadends.
                const auto& other_faithful_abstraction = relevant_faithful_abstractions->at(it->second.get_faithful_abstraction_index());
                const auto& other_state = other_faithful_abstraction.get_graph().get_vertices().at(it->second.get_faithful_abstract_state_index());
                assert(faithful_abstraction.is_goal_state(state.get_index()) == other_faithful_abstraction.is_goal_state(other_state.get_index()));
                assert(faithful_abstraction.is_deadend_state(state.get_index()) == other_faithful_abstraction.is_deadend_state(other_state.get_index()));
            }
            else
            {
                // Create a new global state and add it to global mapping.
                certificate_to_global_state.emplace(
                    mimir::get_certificate(state),
                    states.emplace_back(state.get_index(), certificate_to_global_state.size(), abstraction_index, state.get_index()));
                ++num_non_isomorphic_states;
            }
        }

        relevant_faithful_abstractions->push_back(std::move(faithful_abstraction));

        abstractions.push_back(GlobalFaithfulAbstraction(options.fa_options.mark_true_goal_literals,
                                                         options.fa_options.use_unit_cost_one,
                                                         abstraction_index,
                                                         std::const_pointer_cast<const FaithfulAbstractionList>(relevant_faithful_abstractions),
                                                         std::move(states),
                                                         num_isomorphic_states,
                                                         num_non_isomorphic_states));
        ++abstraction_index;
    }

    return abstractions;
}

/**
 * Abstraction functionality
 */

StateIndex GlobalFaithfulAbstraction::get_abstract_state_index(State concrete_state) const
{
    return m_abstractions->at(m_index).get_abstract_state_index(concrete_state);
}

StateIndex GlobalFaithfulAbstraction::get_abstract_state_index(StateIndex global_state_index) const
{
    return m_global_state_index_to_state_index.at(global_state_index);
}

/**
 * Extended functionality
 */

template<IsTraversalDirection Direction>
DistanceList GlobalFaithfulAbstraction::compute_shortest_distances_from_states(const StateIndexList& abstract_states) const
{
    return m_abstractions->at(m_index).compute_shortest_distances_from_states<Direction>(abstract_states);
}

template DistanceList GlobalFaithfulAbstraction::compute_shortest_distances_from_states<ForwardTraversal>(const StateIndexList& abstract_states) const;
template DistanceList GlobalFaithfulAbstraction::compute_shortest_distances_from_states<BackwardTraversal>(const StateIndexList& abstract_states) const;

template<IsTraversalDirection Direction>
DistanceMatrix GlobalFaithfulAbstraction::compute_pairwise_shortest_state_distances() const
{
    return m_abstractions->at(m_index).compute_pairwise_shortest_state_distances<Direction>();
}

template DistanceMatrix GlobalFaithfulAbstraction::compute_pairwise_shortest_state_distances<ForwardTraversal>() const;
template DistanceMatrix GlobalFaithfulAbstraction::compute_pairwise_shortest_state_distances<BackwardTraversal>() const;

/**
 * Getters
 */

/* Meta data */
Problem GlobalFaithfulAbstraction::get_problem() const { return m_abstractions->at(m_index).get_problem(); }

bool GlobalFaithfulAbstraction::get_mark_true_goal_literals() const { return m_mark_true_goal_literals; }

bool GlobalFaithfulAbstraction::get_use_unit_cost_one() const { return m_use_unit_cost_one; }

AbstractionIndex GlobalFaithfulAbstraction::get_index() const { return m_index; }

/* Memory */
const std::shared_ptr<PDDLFactories>& GlobalFaithfulAbstraction::get_pddl_factories() const { return m_abstractions->at(m_index).get_pddl_factories(); }

const std::shared_ptr<IApplicableActionGenerator>& GlobalFaithfulAbstraction::get_aag() const { return m_abstractions->at(m_index).get_aag(); }

const std::shared_ptr<StateRepository>& GlobalFaithfulAbstraction::get_ssg() const { return m_abstractions->at(m_index).get_ssg(); }

const FaithfulAbstractionList& GlobalFaithfulAbstraction::get_abstractions() const { return *m_abstractions; }

/* Graph */
const typename GlobalFaithfulAbstraction::GraphType& GlobalFaithfulAbstraction::get_graph() const { return m_abstractions->at(m_index).get_graph(); }

/* States */
const GlobalFaithfulAbstractStateList& GlobalFaithfulAbstraction::get_states() const { return m_states; }

template<IsTraversalDirection Direction>
std::ranges::subrange<typename GlobalFaithfulAbstraction::AdjacentVertexConstIteratorType<Direction>>
GlobalFaithfulAbstraction::get_adjacent_states(StateIndex state) const
{
    return get_graph().get_adjacent_vertices<Direction>(state);
}

template std::ranges::subrange<typename GlobalFaithfulAbstraction::AdjacentVertexConstIteratorType<ForwardTraversal>>
GlobalFaithfulAbstraction::get_adjacent_states<ForwardTraversal>(StateIndex state) const;
template std::ranges::subrange<typename GlobalFaithfulAbstraction::AdjacentVertexConstIteratorType<BackwardTraversal>>
GlobalFaithfulAbstraction::get_adjacent_states<BackwardTraversal>(StateIndex state) const;

template<IsTraversalDirection Direction>
std::ranges::subrange<typename GlobalFaithfulAbstraction::AdjacentVertexIndexConstIteratorType<Direction>>
GlobalFaithfulAbstraction::get_adjacent_state_indices(StateIndex state) const
{
    return get_graph().get_adjacent_vertex_indices<Direction>(state);
}

template std::ranges::subrange<typename GlobalFaithfulAbstraction::AdjacentVertexIndexConstIteratorType<ForwardTraversal>>
GlobalFaithfulAbstraction::get_adjacent_state_indices<ForwardTraversal>(StateIndex state) const;
template std::ranges::subrange<typename GlobalFaithfulAbstraction::AdjacentVertexIndexConstIteratorType<BackwardTraversal>>
GlobalFaithfulAbstraction::get_adjacent_state_indices<BackwardTraversal>(StateIndex state) const;

const StateMap<StateIndex>& GlobalFaithfulAbstraction::get_concrete_to_abstract_state() const
{
    return m_abstractions->at(m_index).get_concrete_to_abstract_state();
}

const std::unordered_map<StateIndex, StateIndex>& GlobalFaithfulAbstraction::get_global_state_index_to_state_index() const
{
    return m_global_state_index_to_state_index;
}

StateIndex GlobalFaithfulAbstraction::get_initial_state() const { return m_abstractions->at(m_index).get_initial_state(); }

const StateIndexSet& GlobalFaithfulAbstraction::get_goal_states() const { return m_abstractions->at(m_index).get_goal_states(); }

const StateIndexSet& GlobalFaithfulAbstraction::get_deadend_states() const { return m_abstractions->at(m_index).get_deadend_states(); }

size_t GlobalFaithfulAbstraction::get_num_states() const { return get_states().size(); }

size_t GlobalFaithfulAbstraction::get_num_goal_states() const { return get_goal_states().size(); }

size_t GlobalFaithfulAbstraction::get_num_deadend_states() const { return get_deadend_states().size(); }

bool GlobalFaithfulAbstraction::is_goal_state(StateIndex state) const { return get_goal_states().count(state); }

bool GlobalFaithfulAbstraction::is_deadend_state(StateIndex state) const { return get_deadend_states().count(state); }

bool GlobalFaithfulAbstraction::is_alive_state(StateIndex state) const { return !(get_goal_states().count(state) || get_deadend_states().count(state)); }

size_t GlobalFaithfulAbstraction::get_num_isomorphic_states() const { return m_num_isomorphic_states; }

size_t GlobalFaithfulAbstraction::get_num_non_isomorphic_states() const { return m_num_non_isomorphic_states; }

/* Transitions */
const GroundActionsEdgeList& GlobalFaithfulAbstraction::get_transitions() const { return get_graph().get_edges(); }

template<IsTraversalDirection Direction>
std::ranges::subrange<typename GlobalFaithfulAbstraction::AdjacentEdgeConstIteratorType<Direction>>
GlobalFaithfulAbstraction::get_adjacent_transitions(StateIndex state) const
{
    return get_graph().get_adjacent_edges<Direction>(state);
}

template std::ranges::subrange<typename GlobalFaithfulAbstraction::AdjacentEdgeConstIteratorType<ForwardTraversal>>
GlobalFaithfulAbstraction::get_adjacent_transitions<ForwardTraversal>(StateIndex state) const;
template std::ranges::subrange<typename GlobalFaithfulAbstraction::AdjacentEdgeConstIteratorType<BackwardTraversal>>
GlobalFaithfulAbstraction::get_adjacent_transitions<BackwardTraversal>(StateIndex state) const;

template<IsTraversalDirection Direction>
std::ranges::subrange<typename GlobalFaithfulAbstraction::AdjacentEdgeIndexConstIteratorType<Direction>>
GlobalFaithfulAbstraction::get_adjacent_transition_indices(StateIndex state) const
{
    return get_graph().get_adjacent_edge_indices<Direction>(state);
}

template std::ranges::subrange<typename GlobalFaithfulAbstraction::AdjacentEdgeIndexConstIteratorType<ForwardTraversal>>
GlobalFaithfulAbstraction::get_adjacent_transition_indices<ForwardTraversal>(StateIndex state) const;
template std::ranges::subrange<typename GlobalFaithfulAbstraction::AdjacentEdgeIndexConstIteratorType<BackwardTraversal>>
GlobalFaithfulAbstraction::get_adjacent_transition_indices<BackwardTraversal>(StateIndex state) const;

TransitionCost GlobalFaithfulAbstraction::get_transition_cost(TransitionIndex transition) const
{
    return (m_use_unit_cost_one) ? 1 : m_abstractions->at(m_index).get_transition_cost(transition);
}

size_t GlobalFaithfulAbstraction::get_num_transitions() const { return m_abstractions->at(m_index).get_num_transitions(); }

/* Distances */
const DistanceList& GlobalFaithfulAbstraction::get_goal_distances() const { return m_abstractions->at(m_index).get_goal_distances(); }

Distance GlobalFaithfulAbstraction::get_goal_distance(StateIndex state) const { return m_abstractions->at(m_index).get_goal_distance(state); }

/* Additional */
const std::map<Distance, StateIndexList>& GlobalFaithfulAbstraction::get_states_by_goal_distance() const
{
    return m_abstractions->at(m_index).get_states_by_goal_distance();
}

/**
 * Pretty printing
 */

std::ostream& operator<<(std::ostream& out, const GlobalFaithfulAbstraction& abstraction)
{
    // 2. Header
    out << "digraph {"
        << "\n"
        << "rankdir=\"LR\""
        << "\n";

    // 3. Draw states
    for (size_t state_index = 0; state_index < abstraction.get_num_states(); ++state_index)
    {
        out << "s" << state_index << "[";

        // goal marking
        if (abstraction.is_goal_state(state_index))
        {
            out << "peripheries=2,";
        }

        // label
        const auto& gfa_state = abstraction.get_states().at(state_index);
        out << "label=\"";
        out << "state_index=" << gfa_state.get_index() << " "
            << "global_state_index = " << gfa_state.get_global_index() << " "
            << "abstraction_index=" << gfa_state.get_faithful_abstraction_index() << " "
            << "abstract_state_index=" << gfa_state.get_faithful_abstract_state_index() << "\n";
        const auto& fa_abstraction = abstraction.get_abstractions().at(gfa_state.get_faithful_abstraction_index());
        for (const auto& state : mimir::get_states(fa_abstraction.get_graph().get_vertices().at(gfa_state.get_faithful_abstract_state_index())))
        {
            out << std::make_tuple(fa_abstraction.get_problem(), state, std::cref(*fa_abstraction.get_pddl_factories())) << "\n";
        }
        out << "\"";  // end label

        out << "]\n";
    }

    // 4. Draw initial state and dangling edge
    out << "Dangling [ label = \"\", style = invis ]\n"
        << "{ rank = same; Dangling }\n"
        << "Dangling -> s" << abstraction.get_initial_state() << "\n";

    // 5. Group states with same distance together
    for (auto it = abstraction.get_states_by_goal_distance().rbegin(); it != abstraction.get_states_by_goal_distance().rend(); ++it)
    {
        const auto& [goal_distance, state_indices] = *it;
        out << "{ rank = same; ";
        for (auto state_index : state_indices)
        {
            out << "s" << state_index;
            if (state_index != state_indices.back())
            {
                out << ",";
            }
        }
        out << "}\n";
    }
    // 6. Draw transitions
    for (const auto& transition : abstraction.get_graph().get_edges())
    {
        // direction
        out << "s" << transition.get_source() << "->"
            << "s" << transition.get_target() << " [";

        // label
        out << "label=\"";
        for (const auto& action : get_actions(transition))
        {
            out << action << "\n";
        }
        out << "\"";  // end label

        out << "]\n";
    }
    out << "}\n";

    return out;
}

}
