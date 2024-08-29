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

#ifndef MIMIR_SEARCH_SEARCH_NODE2_HPP_
#define MIMIR_SEARCH_SEARCH_NODE2_HPP_

#include "cista/containers/tuple.h"
#include "cista/serialization.h"  // Serialization functions
#include "mimir/search/action.hpp"
#include "mimir/search/search_node.hpp"
#include "mimir/search/state.hpp"

namespace mimir
{

/**
 * Flatmemory types
 */

template<typename... SearchNodeProperties>
using CistaSearchNode = cista::tuple<SearchNodeStatus,  //
                                     StateIndex,
                                     GroundActionIndex,
                                     SearchNodeProperties...>;
/**
 * Getters.
 */

template<typename... SearchNodeProperties>
SearchNodeStatus get_status(const CistaSearchNode<SearchNodeProperties...>& node)
{
    return cista::get<0>(node);
}

template<typename... SearchNodeProperties>
StateIndex get_state(const CistaSearchNode<SearchNodeProperties...>& node)
{
    return cista::get<1>(node);
}

template<typename... SearchNodeProperties>
GroundActionIndex get_action(const CistaSearchNode<SearchNodeProperties...>& node)
{
    return cista::get<2>(node);
}

/**
 * Setters
 */

template<typename... SearchNodeProperties>
void set_status(CistaSearchNode<SearchNodeProperties...>& node, SearchNodeStatus status)
{
    cista::get<0>(node) = status;
}

template<typename... SearchNodeProperties>
void set_state(CistaSearchNode<SearchNodeProperties...>& node, StateIndex state)
{
    cista::get<1>(node) = state;
}

template<typename... SearchNodeProperties>
void set_action(CistaSearchNode<SearchNodeProperties...>& node, GroundActionIndex action)
{
    cista::get<2>(node) = action;
}

}

#endif