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
                                               StateIndex,
                                               GroundActionIndex,
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
    /// @brief Create a `SearchNodeBuilder` with given arguments.
    /// @param status
    /// @param parent_state
    /// @param creating_action
    /// @param ...properties
    SearchNodeBuilder(SearchNodeStatus status, StateIndex parent_state, GroundActionIndex creating_action, SearchNodeProperties&&... properties) :
        m_builder(std::forward<SearchNodeStatus>(status),
                  std::forward<StateIndex>(parent_state),
                  std::forward<GroundActionIndex>(creating_action),
                  std::forward<SearchNodeProperties>(properties)...)
    {
        finish();
    }

    /// @brief Default constructor enabled only if all `SearchNodeProperties` are default-constructible
    SearchNodeBuilder() requires(std::default_initializable<SearchNodeProperties>&&...) :
        m_builder(SearchNodeStatus::NEW, std::numeric_limits<StateIndex>::max(), std::numeric_limits<GroundActionIndex>::max(), SearchNodeProperties()...)
    {
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
    void set_parent_state(StateIndex parent_state) { m_builder.template get<1>() = parent_state; }
    void set_creating_action(GroundActionIndex creating_action) { m_builder.template get<2>() = creating_action; }

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

    void set_status(SearchNodeStatus status) { m_view.template mutate<0>(status); }
    void set_parent_state(StateIndex state) { m_view.template mutate<1>(state); }
    void set_creating_action(GroundActionIndex action) { m_view.template mutate<2>(action); }

    template<size_t I>
    void set_property(const typename std::tuple_element<I, std::tuple<SearchNodeProperties...>>::type& property)
    {
        static_assert(I < sizeof...(SearchNodeProperties), "Index out of bounds for SearchNodeProperties");
        m_view.template mutate<I + 3>(property);
    }

    /**
     * Mutable getters
     */

    SearchNodeStatus get_status() { return m_view.template get<0>(); }
    StateIndex get_parent_state() { return m_view.template get<1>(); }
    GroundActionIndex get_creating_action() { return m_view.template get<2>(); }

    template<size_t I>
    auto get_property()
    {
        static_assert(I < sizeof...(SearchNodeProperties), "Index out of bounds for SearchNodeProperties");
        return m_view.template get<I + 3>();
    }

    template<size_t I>
    auto get_property() const
    {
        static_assert(I < sizeof...(SearchNodeProperties), "Index out of bounds for SearchNodeProperties");
        return m_view.template get<I + 3>();
    }
};

template<typename Tag, typename... SearchNodeProperties>
void set_status(SearchNode<Tag, SearchNodeProperties...> search_node, SearchNodeStatus status)
{
    return search_node.set_status(status);
}

template<typename Tag, typename... SearchNodeProperties>
void set_parent_state(SearchNode<Tag, SearchNodeProperties...> search_node, StateIndex parent_state)
{
    return search_node.set_parent_state(parent_state);
}

template<typename Tag, typename... SearchNodeProperties>
void set_creating_action(SearchNode<Tag, SearchNodeProperties...> search_node, GroundActionIndex creating_action)
{
    return search_node.set_creating_action(creating_action);
}

template<typename Tag, typename... SearchNodeProperties>
SearchNodeStatus get_status(SearchNode<Tag, SearchNodeProperties...> search_node)
{
    return search_node.get_status();
}

template<typename Tag, typename... SearchNodeProperties>
StateIndex get_parent_state(SearchNode<Tag, SearchNodeProperties...> search_node)
{
    return search_node.get_parent_state();
}

template<typename Tag, typename... SearchNodeProperties>
GroundActionIndex get_creating_action(SearchNode<Tag, SearchNodeProperties...> search_node)
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
    StateIndex get_parent_state() const { return m_view.template get<1>(); }
    GroundActionIndex get_creating_action() const { return m_view.template get<2>(); }

    template<size_t I>
    auto get_property() const
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
StateIndex get_parent_state(ConstSearchNode<Tag, SearchNodeProperties...> search_node)
{
    return search_node.get_parent_state();
}

template<typename Tag, typename... SearchNodeProperties>
GroundActionIndex get_creating_action(ConstSearchNode<Tag, SearchNodeProperties...> search_node)
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
              const GroundActionList& ground_actions,
              const ConstSearchNode<Tag, SearchNodeProperties...>& search_node,
              GroundActionList& out_plan)
{
    out_plan.clear();
    auto cur_search_node = search_node;

    while (cur_search_node.get_parent_state() != std::numeric_limits<StateIndex>::max())
    {
        assert(cur_search_node.get_creating_action() != std::numeric_limits<GroundActionIndex>::max());

        out_plan.push_back(ground_actions.at(cur_search_node.get_creating_action()));

        cur_search_node = search_nodes.at(cur_search_node.get_parent_state());
    }

    std::reverse(out_plan.begin(), out_plan.end());
}

}

#endif