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

#ifndef MIMIR_SEARCH_SEARCH_NODE_HPP_
#define MIMIR_SEARCH_SEARCH_NODE_HPP_

#include "cista/containers/tuple.h"
#include "cista/serialization.h"
#include "mimir/buffering/vector.h"
#include "mimir/common/types.hpp"
#include "mimir/search/action.hpp"

namespace mimir
{

enum SearchNodeStatus
{
    NEW = 0,
    OPEN = 1,
    CLOSED = 2,
    DEAD_END = 3,
    GOAL = 4,
};

template<typename... SearchNodeProperties>
struct SearchNodeImpl
{
    SearchNodeImpl() = default;
    SearchNodeImpl(SearchNodeStatus status, Index parent_state_index, cista::tuple<SearchNodeProperties...> properties) :
        m_status(status),
        m_parent_state_index(parent_state_index),
        m_properties(properties)
    {
    }

    SearchNodeStatus& get_status() { return m_status; }
    SearchNodeStatus get_status() const { return m_status; }

    Index& get_parent_state() { return m_parent_state_index; }
    Index get_parent_state() const { return m_parent_state_index; }

    template<size_t I>
    auto& get_property()
    {
        return cista::get<I>(m_properties);
    }
    template<size_t I>
    const auto& get_property() const
    {
        return cista::get<I>(m_properties);
    }

    /// @brief
    /// @return
    auto cista_members() { return std::tie(m_status, m_parent_state_index, m_properties); }

    SearchNodeStatus m_status = SearchNodeStatus::NEW;
    Index m_parent_state_index = -1;
    cista::tuple<SearchNodeProperties...> m_properties;
};

// We never work with nullptr SearchNodes so we hide the pointer detail.
template<typename... SearchNodeProperties>
using SearchNode = SearchNodeImpl<SearchNodeProperties...>*;
template<typename... SearchNodeProperties>
using ConstSearchNode = const SearchNodeImpl<SearchNodeProperties...>*;

/**
 * Utility
 */

/// @brief Compute the state trajectory that ends in the the `final_state_index` associated with the `final_search_node`.
/// @tparam ...SearchNodeProperties
/// @param search_nodes are all search nodes.
/// @param final_search_node is the final search node.
/// @param final_state_index is the final state index.
/// @param out_trajectory is the resulting state trajectory that ends in the `final_state_index`
template<typename... SearchNodeProperties>
void extract_state_trajectory(const mimir::buffering::Vector<SearchNodeImpl<SearchNodeProperties...>>& search_nodes,  //
                              ConstSearchNode<SearchNodeProperties...> final_search_node,
                              Index final_state_index,
                              IndexList& out_trajectory)
{
    out_trajectory.clear();
    out_trajectory.push_back(final_state_index);

    auto cur_search_node = final_search_node;

    while (cur_search_node->get_parent_state() != std::numeric_limits<Index>::max())
    {
        out_trajectory.push_back(cur_search_node->get_parent_state());

        cur_search_node = search_nodes.at(cur_search_node->get_parent_state());
    }

    std::reverse(out_trajectory.begin(), out_trajectory.end());
}

}

#endif