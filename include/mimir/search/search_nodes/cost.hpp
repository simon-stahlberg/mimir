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
#include "mimir/search/state.hpp"

#include <cassert>
#include <flatmemory/flatmemory.hpp>

namespace mimir
{

/**
 * Data types
 */
enum SearchNodeStatus
{
    NEW = 0,
    OPEN = 1,
    CLOSED = 2,
    DEAD_END = 3
};

namespace flat
{
/**
 * Flatmemory types
 */
using CostSearchNodeLayout = ::flatmemory::Tuple<SearchNodeStatus, int32_t, int32_t, int32_t>;

using CostSearchNodeBuilder = ::flatmemory::Builder<CostSearchNodeLayout>;
using CostSearchNode = ::flatmemory::View<CostSearchNodeLayout>;
using ConstCostSearchNode = ::flatmemory::ConstView<CostSearchNodeLayout>;
using CostSearchNodeVector = ::flatmemory::FixedSizedTypeVector<CostSearchNodeLayout>;
}

/**
 * Proxy for more meaningful access
 */
class CostSearchNodeBuilder
{
private:
    flat::CostSearchNodeBuilder m_builder;

public:
    CostSearchNodeBuilder() : m_builder() {}
    CostSearchNodeBuilder(flat::CostSearchNodeBuilder builder) : m_builder(std::move(builder)) {}

    void finish() { m_builder.finish(); }
    uint8_t* get_data() { return m_builder.buffer().data(); }
    size_t get_size() { return m_builder.buffer().size(); }
    const flat::CostSearchNodeBuilder& get_flatmemory_builder() { return m_builder; }

    void set_status(SearchNodeStatus status) { m_builder.get<0>() = status; }
    void set_g_value(int32_t g_value) { m_builder.get<1>() = g_value; }
    void set_parent_state_id(int32_t parent_state_id) { m_builder.get<2>() = parent_state_id; }
    void set_creating_action_id(int32_t creating_action_id) { m_builder.get<3>() = creating_action_id; }
};

class CostSearchNode
{
private:
    flat::CostSearchNode m_view;

public:
    CostSearchNode(flat::CostSearchNode view) : m_view(view) {}

    SearchNodeStatus& get_status() { return m_view.get<0>(); }
    int32_t& get_g_value() { return m_view.get<1>(); }
    int32_t& get_parent_state_id() { return m_view.get<2>(); }
    int32_t& get_creating_action_id() { return m_view.get<3>(); }
};

class ConstCostSearchNode
{
private:
    flat::ConstCostSearchNode m_view;

public:
    ConstCostSearchNode(flat::ConstCostSearchNode view) : m_view(view) {}

    SearchNodeStatus get_status() const { return m_view.get<0>(); }
    int32_t get_g_value() const { return m_view.get<1>(); }
    int32_t get_parent_state_id() const { return m_view.get<2>(); }
    int32_t get_creating_action_id() const { return m_view.get<3>(); }
};

}

#endif