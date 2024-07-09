/*
 * Copyright (C) 2024 Till Hofmann
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

#ifndef MIMIR_DATASETS_BOOST_ADAPTER_HPP_
#define MIMIR_DATASETS_BOOST_ADAPTER_HPP_

#include "boost/graph/strong_components.hpp"
#include "mimir/common/grouped_vector.hpp"
#include "mimir/datasets/iterators.hpp"
#include "mimir/datasets/state_space.hpp"
#include "mimir/datasets/transition_interface.hpp"
#include "mimir/datasets/transition_system_interface.hpp"
#include "mimir/search/state.hpp"

#include <boost/graph/graph_concepts.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/property_map/property_map.hpp>
#include <limits>
#include <ranges>

namespace boost
{

/// A tag for a graph that is both a vertex list graph and an incidence graph.
struct vertex_list_and_incidence_graph_tag : public vertex_list_graph_tag, public incidence_graph_tag
{
};

/// Traits for a transition system that are needed for the boost graph library.
template<mimir::IsTransitionSystem TransitionSystem>
struct graph_traits<TransitionSystem>
{
    // boost::GraphConcept
    using vertex_descriptor = mimir::StateIndex;
    using edge_descriptor = mimir::TransitionIndex;
    using directed_category = directed_tag;
    using edge_parallel_category = allow_parallel_edge_tag;
    using traversal_category = vertex_list_and_incidence_graph_tag;
    using edges_size_type = size_t;
    // boost::VertexListGraph
    using vertex_iterator = std::ranges::iterator_t<std::ranges::iota_view<vertex_descriptor, vertex_descriptor>>;
    using vertices_size_type = size_t;
    // boost::IncidenceGraph
    using out_edge_iterator = mimir::ForwardTransitionIndexIterator<typename TransitionSystem::TransitionType>::const_iterator;
    using degree_size_type = size_t;
    // boost::strong_components
    constexpr static vertex_descriptor null_vertex() { return std::numeric_limits<vertex_descriptor>::max(); }
};

}

namespace mimir
{

/* boost::VertexListGraph */

/// @brief Get the vertices of the transition system.
/// @param g the transition system.
/// @return an iterator-range providing access to all the vertices in the transition system.
template<IsTransitionSystem TransitionSystem>
std::pair<typename boost::graph_traits<TransitionSystem>::vertex_iterator, typename boost::graph_traits<TransitionSystem>::vertex_iterator>
vertices(const TransitionSystem& g)
{
    std::ranges::iota_view<typename boost::graph_traits<TransitionSystem>::vertex_descriptor, typename boost::graph_traits<TransitionSystem>::vertex_descriptor>
        range(0, g.get_num_states());
    // Make sure we can return dangling iterators.
    static_assert(std::ranges::borrowed_range<decltype(range)>);
    return std::make_pair(range.begin(), range.end());
}

/// @brief Get the number of vertices in the transition system.
/// @param g the transition system.
/// @return the number of vertices in the transition system.
template<IsTransitionSystem TransitionSystem>
boost::graph_traits<TransitionSystem>::vertices_size_type num_vertices(const TransitionSystem& g)
{
    return g.get_num_states();
}

/* boost::IncidenceGraph */

/// @brief Get the source state of a transition.
/// @param e the transition.
/// @param g the transition system.
/// @return the source state of the transition.
template<IsTransitionSystem TransitionSystem>
typename boost::graph_traits<TransitionSystem>::vertex_descriptor source(const typename boost::graph_traits<TransitionSystem>::edge_descriptor& e,
                                                                         const TransitionSystem& g)
{
    return g.get_transitions()[e].get_source_state();
}

/// @brief Get the target state of a transition.
/// @param e the transition.
/// @param g the transition system.
/// @return the target state of the transition.
template<IsTransitionSystem TransitionSystem>
typename boost::graph_traits<TransitionSystem>::vertex_descriptor target(const typename boost::graph_traits<TransitionSystem>::edge_descriptor& e,
                                                                         const TransitionSystem& g)
{
    return g.get_transitions()[e].get_target_state();
}

/// @brief Get the transitions of the transition system.
/// @param g the transition system.
/// @return an iterator-range providing access to all the forward transitions (i.e., the out edges) in the transition system.
template<IsTransitionSystem TransitionSystem>
std::pair<typename boost::graph_traits<TransitionSystem>::out_edge_iterator, typename boost::graph_traits<TransitionSystem>::out_edge_iterator>
out_edges(typename boost::graph_traits<TransitionSystem>::vertex_descriptor const& u, const TransitionSystem& g)
{
    return { g.get_forward_transition_indices(u).begin(), g.get_forward_transition_indices(u).end() };
}

/// @brief Get the number of out edges of a state.
/// @param u the state.
/// @param g the transition system.
/// @return the number of out edges of the state.
template<IsTransitionSystem TransitionSystem>
boost::graph_traits<TransitionSystem>::degree_size_type out_degree(typename boost::graph_traits<TransitionSystem>::vertex_descriptor const& u,
                                                                   const TransitionSystem& g)
{
    return std::distance(g.get_forward_transition_indices(u).begin(), g.get_forward_transition_indices(u).end());
}

}

// boost::strong_components requires a vertex_index that translates vertex->index, where the index is an index into the vertex list.
// In our case, the vertex is described with a StateIndex and so vertex and index are the same.
// To avoid storing a map of the size of the graph, we provide a custom index that just returns the key.
namespace mimir
{
/// @brief A property map that maps a vertex to its index.
struct IdIsIndexVertexIndex
{
};
}

namespace boost
{

/// @brief Traits for the IdIsIndexVertexIndex property map, required for boost::strong_components.
template<>
struct property_traits<mimir::IdIsIndexVertexIndex>
{
    using value_type = mimir::StateIndex;
    using key_type = mimir::StateIndex;
    using reference = mimir::StateIndex;
    using category = boost::readable_property_map_tag;
};

}

namespace mimir
{

/// @brief Get the index of a state.
/// @param key the state.
/// @return the index of the state, which is just the input key.
inline boost::property_traits<IdIsIndexVertexIndex>::reference get(IdIsIndexVertexIndex, boost::property_traits<IdIsIndexVertexIndex>::key_type key)
{
    return key;
}

/// @brief Wrapper function for boost's strong_components algorithm.
/// @param g the transition system.
/// @return a pair of the number of strong components and a map from state to component.
template<IsTransitionSystem TransitionSystem>
std::pair<typename boost::graph_traits<TransitionSystem>::vertices_size_type,
          std::map<typename boost::graph_traits<TransitionSystem>::vertex_descriptor, typename boost::graph_traits<TransitionSystem>::vertices_size_type>>
strong_components(const TransitionSystem& g)
{
    std::map<StateIndex, size_t> component_map;
    boost::associative_property_map component_map_property(component_map);
    const auto num_components = boost::strong_components(g, component_map_property, boost::vertex_index_map(IdIsIndexVertexIndex()));
    return std::make_pair(num_components, component_map);
}

template<IsTransitionSystem TransitionSystem>
IndexGroupedVector<
    std::pair<typename boost::graph_traits<TransitionSystem>::vertices_size_type, typename boost::graph_traits<TransitionSystem>::vertex_descriptor>>
get_partitioning(typename boost::graph_traits<TransitionSystem>::vertices_size_type num_components,
                 std::map<typename boost::graph_traits<TransitionSystem>::vertex_descriptor, typename boost::graph_traits<TransitionSystem>::vertices_size_type>
                     component_map)
{
    using state_component_pair_t =
        std::pair<typename boost::graph_traits<TransitionSystem>::vertices_size_type, typename boost::graph_traits<TransitionSystem>::vertex_descriptor>;
    auto partitioning = std::vector<state_component_pair_t>();
    for (const auto& [state, component] : component_map)
    {
        partitioning.push_back({ component, state });
    }
    std::sort(std::begin(partitioning), std::end(partitioning));
    return IndexGroupedVector<state_component_pair_t>::create(std::move(partitioning),
                                                              [](const auto& prev, const auto& cur) { return prev.first != cur.first; });
}

/* Assert that the concepts are satisfied */
BOOST_CONCEPT_ASSERT((boost::GraphConcept<StateSpace>) );
BOOST_CONCEPT_ASSERT((boost::VertexListGraphConcept<StateSpace>) );
BOOST_CONCEPT_ASSERT((boost::IncidenceGraphConcept<StateSpace>) );

}

#endif
