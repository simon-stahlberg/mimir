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

#ifndef MIMIR_SEARCH_SEARCH_NODES_COST_HPP_
#define MIMIR_SEARCH_SEARCH_NODES_COST_HPP_

#include "mimir/search/action.hpp"
#include "mimir/search/search_nodes/status.hpp"
#include "mimir/search/state.hpp"

#include <cassert>
#include <flatmemory/flatmemory.hpp>

namespace mimir
{

using FlatUninformedSearchNodeLayout = flatmemory::Tuple<SearchNodeStatus, int32_t, int32_t, int32_t>;

using FlatUninformedSearchNodeBuilder = flatmemory::Builder<FlatUninformedSearchNodeLayout>;
using FlatUninformedSearchNode = flatmemory::View<FlatUninformedSearchNodeLayout>;
using FlatConstUninformedSearchNode = flatmemory::ConstView<FlatUninformedSearchNodeLayout>;

using FlatUninformedSearchNodeVector = flatmemory::FixedSizedTypeVector<FlatUninformedSearchNodeLayout>;

/**
 * Proxy for more meaningful access
 */
class UninformedSearchNodeBuilder
{
private:
    FlatUninformedSearchNodeBuilder m_builder;

public:
    UninformedSearchNodeBuilder() : m_builder() {}
    UninformedSearchNodeBuilder(FlatUninformedSearchNodeBuilder builder) : m_builder(std::move(builder)) {}

    void finish() { m_builder.finish(); }
    uint8_t* get_data() { return m_builder.buffer().data(); }
    size_t get_size() { return m_builder.buffer().size(); }
    FlatUninformedSearchNodeBuilder& get_flatmemory_builder() { return m_builder; }
    const FlatUninformedSearchNodeBuilder& get_flatmemory_builder() const { return m_builder; }

    void set_status(SearchNodeStatus status) { m_builder.get<0>() = status; }
    void set_g_value(int32_t g_value) { m_builder.get<1>() = g_value; }
    void set_parent_state_id(int32_t parent_state_id) { m_builder.get<2>() = parent_state_id; }
    void set_creating_action_id(int32_t creating_action_id) { m_builder.get<3>() = creating_action_id; }
};

class UninformedSearchNode
{
private:
    FlatUninformedSearchNode m_view;

public:
    UninformedSearchNode(FlatUninformedSearchNode view) : m_view(view) {}

    SearchNodeStatus& get_status() { return m_view.get<0>(); }
    int32_t& get_g_value() { return m_view.get<1>(); }
    int32_t& get_parent_state_id() { return m_view.get<2>(); }
    int32_t& get_creating_action_id() { return m_view.get<3>(); }
};

class ConstUninformedCostSearchNode
{
private:
    FlatConstUninformedSearchNode m_view;

public:
    ConstUninformedCostSearchNode(FlatConstUninformedSearchNode view) : m_view(view) {}

    SearchNodeStatus get_status() const { return m_view.get<0>(); }
    int32_t get_g_value() const { return m_view.get<1>(); }
    int32_t get_parent_state_id() const { return m_view.get<2>(); }
    int32_t get_creating_action_id() const { return m_view.get<3>(); }
};

}

#endif