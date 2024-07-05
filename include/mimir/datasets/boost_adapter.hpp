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

#include "mimir/datasets/iterators.hpp"
#include "mimir/datasets/state_space.hpp"
#include "mimir/datasets/transition_interface.hpp"
#include "mimir/datasets/transition_system_interface.hpp"

#include <boost/graph/graph_concepts.hpp>
#include <boost/graph/graph_traits.hpp>
#include <ranges>

namespace mimir
{
struct StateIndexRangeView : std::ranges::iota_view<StateIndex, StateIndex>
{
    using std::ranges::iota_view<StateIndex, StateIndex>::iota_view;
};
}

template<>
inline constexpr bool std::ranges::enable_borrowed_range<mimir::StateIndexRangeView> = true;

namespace boost
{

struct vertex_list_and_incidence_graph_tag : public vertex_list_graph_tag, public incidence_graph_tag
{
};

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
    using vertex_iterator = std::ranges::iterator_t<mimir::StateIndexRangeView>;
    using vertices_size_type = size_t;
    // boost::IncidenceGraph
    using out_edge_iterator = mimir::ForwardTransitionIndexIterator<typename TransitionSystem::TransitionType>::const_iterator;
    using degree_size_type = size_t;
};

}

namespace mimir
{

/* boost::VertexListGraph */

template<IsTransitionSystem TransitionSystem>
std::pair<typename boost::graph_traits<TransitionSystem>::vertex_iterator, typename boost::graph_traits<TransitionSystem>::vertex_iterator>
vertices(const TransitionSystem& g)
{
    StateIndexRangeView range(0, g.get_num_states());
    // Make sure we can return dangling iterators.
    static_assert(std::ranges::borrowed_range<StateIndexRangeView>);
    return std::make_pair(range.begin(), range.end());
}

template<IsTransitionSystem TransitionSystem>
boost::graph_traits<TransitionSystem>::vertices_size_type num_vertices(const TransitionSystem& g)
{
    return g.get_num_states();
}

/* boost::IncidenceGraph */

template<IsTransitionSystem TransitionSystem>
typename boost::graph_traits<TransitionSystem>::vertex_descriptor source(const typename boost::graph_traits<TransitionSystem>::edge_descriptor& e,
                                                                         const TransitionSystem& g)
{
    return g.get_transitions()[e].get_source_state();
}

template<IsTransitionSystem TransitionSystem>
typename boost::graph_traits<TransitionSystem>::vertex_descriptor target(const typename boost::graph_traits<TransitionSystem>::edge_descriptor& e,
                                                                         const TransitionSystem& g)
{
    return g.get_transitions()[e].get_target_state();
}

template<IsTransitionSystem TransitionSystem>
std::pair<typename boost::graph_traits<TransitionSystem>::out_edge_iterator, typename boost::graph_traits<TransitionSystem>::out_edge_iterator>
out_edges(typename boost::graph_traits<TransitionSystem>::vertex_descriptor const& u, const TransitionSystem& g)
{
    return { g.get_forward_transition_indices(u).begin(), g.get_forward_transition_indices(u).end() };
}

template<IsTransitionSystem TransitionSystem>
boost::graph_traits<TransitionSystem>::degree_size_type out_degree(typename boost::graph_traits<TransitionSystem>::vertex_descriptor const& u,
                                                                   const TransitionSystem& g)
{
    return std::distance(g.get_forward_transition_indices(u).begin(), g.get_forward_transition_indices(u).end());
}

/* Assert that the concepts are satisfied */
BOOST_CONCEPT_ASSERT((boost::GraphConcept<StateSpace>) );
BOOST_CONCEPT_ASSERT((boost::VertexListGraphConcept<StateSpace>) );
BOOST_CONCEPT_ASSERT((boost::IncidenceGraphConcept<StateSpace>) );
}

#endif
