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

#include "mimir/datasets/state_space.hpp"
#include "mimir/datasets/transition_system.hpp"

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
template<mimir::IsTransitionSystem TransitionSystem>
struct graph_traits<TransitionSystem>
{
    using vertex_descriptor = mimir::StateIndex;
    using edge_descriptor = mimir::TransitionIndex;
    using directed_category = directed_tag;
    using edge_parallel_category = allow_parallel_edge_tag;
    using traversal_category = vertex_list_graph_tag;
    using vertices_size_type = size_t;
    using edges_size_type = size_t;
    using vertex_iterator = std::ranges::iterator_t<mimir::StateIndexRangeView>;
};

}

namespace mimir
{

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

BOOST_CONCEPT_ASSERT((boost::GraphConcept<StateSpace>) );
BOOST_CONCEPT_ASSERT((boost::VertexListGraphConcept<StateSpace>) );
}

#endif
