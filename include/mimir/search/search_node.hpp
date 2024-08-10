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

#include "mimir/search/action.hpp"
#include "mimir/search/state.hpp"

#include <cassert>
#include <flatmemory/flatmemory.hpp>

namespace mimir
{

enum SearchNodeStatus
{
    NEW = 0,
    OPEN = 1,
    CLOSED = 2,
    DEAD_END = 3,
};

/**
 * Flatmemory types
 */

template<typename... SearchNodeProperties>
using FlatSearchNodeLayout = flatmemory::Tuple<SearchNodeStatus,  //
                                               std::optional<State>,
                                               std::optional<GroundAction>,
                                               SearchNodeProperties...>;

template<typename... SearchNodeProperties>
using FlatSearchNodeBuilder = flatmemory::Builder<FlatSearchNodeLayout<SearchNodeProperties...>>;
template<typename... SearchNodeProperties>
using FlatSearchNode = flatmemory::View<FlatSearchNodeLayout<SearchNodeProperties...>>;
template<typename... SearchNodeProperties>
using FlatConstSearchNode = flatmemory::ConstView<FlatSearchNodeLayout<SearchNodeProperties...>>;

template<typename... SearchNodeProperties>
using FlatSearchNodeVector = flatmemory::FixedSizedTypeVector<FlatSearchNodeLayout<SearchNodeProperties...>>;

/**
 * Mimir types
 */

/// @brief `SearchNodeBuilder` is a wrapper around `FlatSearchNodeBuilder` to create byte sequences
/// that can be zero-cost deserialized with a `SearchNode` or `ConstSearchNode`.
/// @tparam ...SearchNodeProperties the properties stored in a search node which must be serializable by flatmemory.
/// For example, BrFs stores a uint32_t to track the g-value, and AStar stores two doubles to track the g-value and h-value.
template<typename Tag, typename... SearchNodeProperties>
class SearchNodeBuilder
{
public:
    /// Create a default initialized `SearchNodeBuilder`.
    SearchNodeBuilder() : m_builder() { finish(); }

    /// @brief Create a `SearchNodeBuilder` with given arguments.
    /// @param status
    /// @param parent_state
    /// @param creating_action
    /// @param ...properties
    SearchNodeBuilder(SearchNodeStatus status,
                      std::optional<State> parent_state,
                      std::optional<GroundAction> creating_action,
                      SearchNodeProperties&&... properties)
    {
        set_status(status);
        set_parent_state(parent_state);
        set_creating_action(creating_action);
        set_properties(std::make_index_sequence<sizeof...(SearchNodeProperties)> {}, std::forward<SearchNodeProperties>(properties)...);
        finish();
    }

    void finish() { m_builder.finish(); }

    /**
     * Mutable getters
     */

    uint8_t* get_data() { return m_builder.buffer().data(); }
    size_t get_size() { return m_builder.buffer().size(); }
    FlatSearchNodeBuilder<SearchNodeProperties...>& get_flatmemory_builder() { return m_builder; }
    const FlatSearchNodeBuilder<SearchNodeProperties...>& get_flatmemory_builder() const { return m_builder; }

    void set_status(SearchNodeStatus status) { m_builder.template get<0>() = status; }
    void set_parent_state(std::optional<State> parent_state) { m_builder.template get<1>() = parent_state; }
    void set_creating_action(std::optional<GroundAction> creating_action) { m_builder.template get<2>() = creating_action; }

    /// @brief Return a reference to the I-th `SearchNodeProperties`.
    /// @tparam I the index of the property in the parameter pack.
    /// @return a reference to the I-th property.
    template<size_t I>
    auto& get_property()
    {
        static_assert(I < sizeof...(SearchNodeProperties), "Index out of bounds for SearchNodeProperties");
        return m_builder.template get<I + 3>();
    }

private:
    FlatSearchNodeBuilder<SearchNodeProperties...> m_builder;

    // Helper function to set properties using an index sequence.
    template<std::size_t... Is>
    void set_properties(std::index_sequence<Is...>, SearchNodeProperties&&... properties)
    {
        // This uses a fold expression to set each property.
        ((m_builder.template get<3 + Is>() = std::forward<SearchNodeProperties>(properties)), ...);
    }
};

/// @brief `SearchNode` is a mutable wrapper around `FlatSearchNode` to read and write the data.
template<typename Tag, typename... SearchNodeProperties>
class SearchNode
{
private:
    FlatSearchNode<SearchNodeProperties...> m_view;

public:
    SearchNode(FlatSearchNode<SearchNodeProperties...> view) : m_view(view) {}

    /**
     * Setters
     */

    void set_status(SearchNodeStatus status) { m_view.template get<0>() = status; }
    void set_parent_state(State state) { m_view.template get<1>() = state; }
    void set_creating_action(GroundAction action) { m_view.template get<2>() = action; }

    template<size_t I>
    void set_property(const typename std::tuple_element<I, std::tuple<SearchNodeProperties...>>::type& property)
    {
        static_assert(I < sizeof...(SearchNodeProperties), "Index out of bounds for SearchNodeProperties");
        m_view.template get<I + 3>() = property;
    }

    /**
     * Mutable getters
     */

    SearchNodeStatus& get_status() { return m_view.template get<0>(); }
    std::optional<State>& get_parent_state() { return m_view.template get<1>(); }
    std::optional<GroundAction>& get_creating_action() { return m_view.template get<2>(); }

    template<size_t I>
    auto& get_property()
    {
        static_assert(I < sizeof...(SearchNodeProperties), "Index out of bounds for SearchNodeProperties");
        return m_view.template get<I + 3>();
    }

    // No immutable getters because they are unsafe anyways since constness of
    // flatmemory views can be cast away by simply copying the view into a non const object.
};

template<typename Tag, typename... SearchNodeProperties>
void set_status(SearchNode<Tag, SearchNodeProperties...> search_node, SearchNodeStatus status)
{
    return search_node.set_status(status);
}

template<typename Tag, typename... SearchNodeProperties>
void set_parent_state(SearchNode<Tag, SearchNodeProperties...> search_node, State parent_state)
{
    return search_node.set_parent_state(parent_state);
}

template<typename Tag, typename... SearchNodeProperties>
void set_creating_action(SearchNode<Tag, SearchNodeProperties...> search_node, GroundAction creating_action)
{
    return search_node.set_creating_action(creating_action);
}

template<typename Tag, typename... SearchNodeProperties>
SearchNodeStatus& get_status(SearchNode<Tag, SearchNodeProperties...> search_node)
{
    return search_node.get_status();
}

template<typename Tag, typename... SearchNodeProperties>
std::optional<State>& get_parent_state(SearchNode<Tag, SearchNodeProperties...> search_node)
{
    return search_node.get_parent_state();
}

template<typename Tag, typename... SearchNodeProperties>
std::optional<GroundAction>& get_creating_action(SearchNode<Tag, SearchNodeProperties...> search_node)
{
    return search_node.get_creating_action();
}

/// @brief `ConstSearchNode` is a immutable wrapper around `FlatConstSearchNode` to read the data.
template<typename Tag, typename... SearchNodeProperties>
class ConstSearchNode
{
private:
    FlatConstSearchNode<SearchNodeProperties...> m_view;

public:
    ConstSearchNode(FlatConstSearchNode<SearchNodeProperties...> view) : m_view(view) {}

    /**
     * Immutable getters
     */

    SearchNodeStatus get_status() const { return m_view.template get<0>(); }
    std::optional<State> get_parent_state() const { return m_view.template get<1>(); }
    std::optional<GroundAction> get_creating_action() const { return m_view.template get<2>(); }

    template<size_t I>
    const auto& get_property() const
    {
        static_assert(I < sizeof...(SearchNodeProperties), "Index out of bounds for SearchNodeProperties");
        return m_view.template get<I + 3>();
    }
};

template<typename Tag, typename... SearchNodeProperties>
SearchNodeStatus get_status(ConstSearchNode<Tag, SearchNodeProperties...> search_node)
{
    return search_node.get_status();
}

template<typename Tag, typename... SearchNodeProperties>
std::optional<State> get_parent_state(ConstSearchNode<Tag, SearchNodeProperties...> search_node)
{
    return search_node.get_parent_state();
}

template<typename Tag, typename... SearchNodeProperties>
std::optional<GroundAction> get_creating_action(ConstSearchNode<Tag, SearchNodeProperties...> search_node)
{
    return search_node.get_creating_action();
}

/// @brief Compute the plan consisting of ground actions by collecting the creating actions
///        and reversing them.
/// @param search_nodes The collection of all search nodes.
/// @param search_node The search node from which to start backtracking.
/// @param[out] out_plan The sequence of ground actions that leads from the initial state to
///                      the to the state underlying the search node.
template<typename Tag, typename... SearchNodeProperties>
void set_plan(const FlatSearchNodeVector<SearchNodeProperties...>& search_nodes,  //
              const ConstSearchNode<Tag, SearchNodeProperties...>& search_node,
              GroundActionList& out_plan)
{
    out_plan.clear();
    auto cur_search_node = search_node;

    while (cur_search_node.get_parent_state().has_value())
    {
        assert(cur_search_node.get_creating_action().has_value());

        out_plan.push_back(cur_search_node.get_creating_action().value());

        cur_search_node = ConstSearchNode<Tag, SearchNodeProperties...>(search_nodes.at(cur_search_node.get_parent_state().value().get_index()));
    }

    std::reverse(out_plan.begin(), out_plan.end());
}

}

#endif