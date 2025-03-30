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

#include "mimir/buffering/vector.h"
#include "mimir/common/types.hpp"

namespace mimir::search
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

    /// @brief Return all members to make them serializable by cista.
    /// @return a tuple of references to all members.
    auto cista_members() { return std::tie(m_status, m_parent_state_index, m_properties); }

    SearchNodeStatus m_status = SearchNodeStatus::NEW;
    Index m_parent_state_index = std::numeric_limits<Index>::max();
    cista::tuple<SearchNodeProperties...> m_properties;
};

template<typename... SearchNodeProperties>
using SearchNode = SearchNodeImpl<SearchNodeProperties...>*;
template<typename... SearchNodeProperties>
using ConstSearchNode = const SearchNodeImpl<SearchNodeProperties...>*;

template<typename... SearchNodeProperties>
using SearchNodeImplVector = mimir::buffering::Vector<SearchNodeImpl<SearchNodeProperties...>>;

}

#endif