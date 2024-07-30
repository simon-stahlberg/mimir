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
#include "mimir/search/search_nodes/interface.hpp"
#include "mimir/search/search_nodes/status.hpp"
#include "mimir/search/state.hpp"

#include <cassert>
#include <flatmemory/flatmemory.hpp>

namespace mimir
{

using FlatInformedSearchNodeLayout = flatmemory::Tuple<SearchNodeStatus,  //
                                                       double,
                                                       std::optional<State>,
                                                       std::optional<GroundAction>,
                                                       FlatDoubleListLayout>;

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
    void set_g_value(double g_value) { m_builder.get<1>() = g_value; }
    void set_parent_state(std::optional<State> parent_state) { m_builder.get<2>() = parent_state; }
    void set_creating_action(std::optional<GroundAction> creating_action) { m_builder.get<3>() = creating_action; }
    void set_h_value(size_t pos, double h_value) { m_builder.get<4>().at(pos) = h_value; }

    FlatDoubleListBuilder& get_h_values() { return m_builder.get<4>(); }
};

class InformedSearchNode
{
public:
    using Layout = FlatInformedSearchNodeLayout;

private:
    FlatInformedSearchNode m_view;

public:
    InformedSearchNode(FlatInformedSearchNode view) : m_view(view) {}

    SearchNodeStatus& get_status() { return m_view.get<0>(); }
    double& get_g_value() { return m_view.get<1>(); }
    std::optional<State>& get_parent_state() { return m_view.get<2>(); }
    std::optional<GroundAction>& get_creating_action() { return m_view.get<3>(); }
    FlatDoubleList get_h_values() { return m_view.get<4>(); }
};

static_assert(IsPathExtractableSearchNode<InformedSearchNode>);

class ConstInformedSearchNode
{
public:
    using Layout = FlatInformedSearchNodeLayout;

private:
    FlatConstInformedSearchNode m_view;

public:
    ConstInformedSearchNode(FlatConstInformedSearchNode view) : m_view(view) {}

    SearchNodeStatus get_status() const { return m_view.get<0>(); }
    double get_g_value() const { return m_view.get<1>(); }
    std::optional<State> get_parent_state() const { return m_view.get<2>(); }
    std::optional<GroundAction> get_creating_action() const { return m_view.get<3>(); }
    FlatConstDoubleList get_h_values() { return m_view.get<4>(); }
};

static_assert(IsPathExtractableSearchNode<ConstInformedSearchNode>);

}

#endif