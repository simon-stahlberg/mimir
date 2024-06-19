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

#ifndef MIMIR_SEARCH_SEARCH_NODES_INFORMED_HPP_
#define MIMIR_SEARCH_SEARCH_NODES_INFORMED_HPP_

#include "mimir/search/action.hpp"
#include "mimir/search/search_nodes/status.hpp"
#include "mimir/search/state.hpp"

#include <cassert>
#include <flatmemory/flatmemory.hpp>

namespace mimir
{

using FlatInformedSearchNodeLayout = flatmemory::Tuple<SearchNodeStatus, int32_t, int32_t, int32_t, flatmemory::Vector<double>>;

using FlatInformedSearchNodeBuilder = flatmemory::Builder<FlatInformedSearchNodeLayout>;
using FlatInformedSearchNode = flatmemory::View<FlatInformedSearchNodeLayout>;
using FlatConstInformedSearchNode = flatmemory::ConstView<FlatInformedSearchNodeLayout>;

using FlatInformedSearchNodeVector = flatmemory::FixedSizedTypeVector<FlatInformedSearchNodeLayout>;

/**
 * Proxy for more meaningful access
 */
class InformedSearchNodeBuilder
{
private:
    FlatInformedSearchNodeBuilder m_builder;

public:
    InformedSearchNodeBuilder() : m_builder() {}
    InformedSearchNodeBuilder(FlatInformedSearchNodeBuilder builder) : m_builder(std::move(builder)) {}

    void finish() { m_builder.finish(); }
    uint8_t* get_data() { return m_builder.buffer().data(); }
    size_t get_size() { return m_builder.buffer().size(); }
    FlatInformedSearchNodeBuilder& get_flatmemory_builder() { return m_builder; }
    const FlatInformedSearchNodeBuilder& get_flatmemory_builder() const { return m_builder; }

    void set_status(SearchNodeStatus status) { m_builder.get<0>() = status; }
    void set_g_value(int32_t g_value) { m_builder.get<1>() = g_value; }
    void set_parent_state_id(int32_t parent_state_id) { m_builder.get<2>() = parent_state_id; }
    void set_creating_action_id(int32_t creating_action_id) { m_builder.get<3>() = creating_action_id; }
    void set_h_value(size_t pos, double h_value) { m_builder.get<4>().at(pos) = h_value; }

    std::vector<double>& get_h_values() { return m_builder.get<4>(); }
};

class InformedSearchNode
{
private:
    FlatInformedSearchNode m_view;

public:
    InformedSearchNode(FlatInformedSearchNode view) : m_view(view) {}

    SearchNodeStatus& get_status() { return m_view.get<0>(); }
    int32_t& get_g_value() { return m_view.get<1>(); }
    int32_t& get_parent_state_id() { return m_view.get<2>(); }
    int32_t& get_creating_action_id() { return m_view.get<3>(); }
};

class ConstInformedCostSearchNode
{
private:
    FlatConstInformedSearchNode m_view;

public:
    ConstInformedCostSearchNode(FlatConstInformedSearchNode view) : m_view(view) {}

    SearchNodeStatus get_status() const { return m_view.get<0>(); }
    int32_t get_g_value() const { return m_view.get<1>(); }
    int32_t get_parent_state_id() const { return m_view.get<2>(); }
    int32_t get_creating_action_id() const { return m_view.get<3>(); }
};

}

#endif