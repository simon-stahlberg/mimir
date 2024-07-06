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

#ifndef MIMIR_GRAPHS_DIGRAPH_HPP_
#define MIMIR_GRAPHS_DIGRAPH_HPP_

#include <ranges>
#include <span>
#include <vector>

namespace mimir
{

using DigraphEdgeIndex = int;
using DigraphEdgeWeight = double;
using DigraphVertexIndex = int;

class DigraphEdge
{
private:
    DigraphEdgeIndex m_index;
    DigraphVertexIndex m_source;
    DigraphVertexIndex m_target;
    DigraphEdgeWeight m_weight;

public:
    DigraphEdge(DigraphEdgeIndex index, DigraphVertexIndex source, DigraphVertexIndex target, DigraphEdgeWeight weight);

    bool operator==(const DigraphEdge& other) const;
    size_t hash() const;

    DigraphEdgeIndex get_index() const;
    DigraphVertexIndex get_source() const;
    DigraphVertexIndex get_target() const;
    DigraphEdgeWeight get_weight() const;
};

using DigraphEdgeList = std::vector<DigraphEdge>;

class DigraphTargetIndexIterator
{
private:
    DigraphVertexIndex m_source;
    std::span<const DigraphEdge> m_edges;

public:
    DigraphTargetIndexIterator(DigraphVertexIndex source, std::span<const DigraphEdge> edges);

    class const_iterator
    {
    private:
        DigraphVertexIndex m_source;
        size_t m_pos;
        std::span<const DigraphEdge> m_edges;

        void advance();

    public:
        using difference_type = int;
        using value_type = DigraphVertexIndex;
        using pointer = value_type*;
        using reference = value_type&;
        using iterator_category = std::forward_iterator_tag;

        const_iterator();
        const_iterator(DigraphVertexIndex source, std::span<const DigraphEdge> edges, bool begin);
        [[nodiscard]] value_type operator*() const;
        const_iterator& operator++();
        const_iterator operator++(int);
        [[nodiscard]] bool operator==(const const_iterator& other) const;
        [[nodiscard]] bool operator!=(const const_iterator& other) const;
    };

    [[nodiscard]] const_iterator begin() const;
    [[nodiscard]] const_iterator end() const;
};

static_assert(std::ranges::forward_range<DigraphTargetIndexIterator>);

class DigraphSourceIndexIterator
{
private:
    DigraphVertexIndex m_target;
    std::span<const DigraphEdge> m_edges;

public:
    DigraphSourceIndexIterator(DigraphVertexIndex target, std::span<const DigraphEdge> edges);

    class const_iterator
    {
    private:
        DigraphVertexIndex m_target;
        size_t m_pos;
        std::span<const DigraphEdge> m_edges;

        void advance();

    public:
        using difference_type = int;
        using value_type = DigraphVertexIndex;
        using pointer = value_type*;
        using reference = value_type&;
        using iterator_category = std::forward_iterator_tag;

        const_iterator();
        const_iterator(DigraphVertexIndex target, std::span<const DigraphEdge> edges, bool begin);
        [[nodiscard]] value_type operator*() const;
        const_iterator& operator++();
        const_iterator operator++(int);
        [[nodiscard]] bool operator==(const const_iterator& other) const;
        [[nodiscard]] bool operator!=(const const_iterator& other) const;
    };

    [[nodiscard]] const_iterator begin() const;
    [[nodiscard]] const_iterator end() const;
};

static_assert(std::ranges::forward_range<DigraphSourceIndexIterator>);

class DigraphForwardEdgeIndexIterator
{
private:
    DigraphVertexIndex m_source;
    std::span<const DigraphEdge> m_edges;

public:
    DigraphForwardEdgeIndexIterator(DigraphVertexIndex source, std::span<const DigraphEdge> edges);

    class const_iterator
    {
    private:
        DigraphVertexIndex m_source;
        size_t m_pos;
        std::span<const DigraphEdge> m_edges;

        void advance();

    public:
        using difference_type = int;
        using value_type = DigraphEdgeIndex;
        using pointer = value_type*;
        using reference = value_type&;
        using iterator_category = std::forward_iterator_tag;

        const_iterator();
        const_iterator(DigraphVertexIndex source, std::span<const DigraphEdge> edges, bool begin);
        [[nodiscard]] value_type operator*() const;
        const_iterator& operator++();
        const_iterator operator++(int);
        [[nodiscard]] bool operator==(const const_iterator& other) const;
        [[nodiscard]] bool operator!=(const const_iterator& other) const;
    };

    [[nodiscard]] const_iterator begin() const;
    [[nodiscard]] const_iterator end() const;
};

static_assert(std::ranges::forward_range<DigraphForwardEdgeIndexIterator>);

class DigraphBackwardEdgeIndexIterator
{
private:
    DigraphVertexIndex m_target;
    std::span<const DigraphEdge> m_edges;

public:
    DigraphBackwardEdgeIndexIterator(DigraphVertexIndex target, std::span<const DigraphEdge> edges);

    class const_iterator
    {
    private:
        DigraphVertexIndex m_target;
        size_t m_pos;
        std::span<const DigraphEdge> m_edges;

        void advance();

    public:
        using difference_type = int;
        using value_type = DigraphEdgeIndex;
        using pointer = value_type*;
        using reference = value_type&;
        using iterator_category = std::forward_iterator_tag;

        const_iterator();
        const_iterator(DigraphVertexIndex target, std::span<const DigraphEdge> edges, bool begin);
        [[nodiscard]] value_type operator*() const;
        const_iterator& operator++();
        const_iterator operator++(int);
        [[nodiscard]] bool operator==(const const_iterator& other) const;
        [[nodiscard]] bool operator!=(const const_iterator& other) const;
    };

    [[nodiscard]] const_iterator begin() const;
    [[nodiscard]] const_iterator end() const;
};

static_assert(std::ranges::forward_range<DigraphBackwardEdgeIndexIterator>);

class DigraphForwardEdgeIterator
{
private:
    DigraphVertexIndex m_source;
    std::span<const DigraphEdge> m_edges;

public:
    DigraphForwardEdgeIterator(DigraphVertexIndex source, std::span<const DigraphEdge> edges);

    class const_iterator
    {
    private:
        DigraphVertexIndex m_source;
        size_t m_pos;
        std::span<const DigraphEdge> m_edges;

        void advance();

    public:
        using difference_type = int;
        using value_type = const DigraphEdge;
        using pointer = value_type*;
        using reference = value_type&;
        using iterator_category = std::forward_iterator_tag;

        const_iterator();
        const_iterator(DigraphVertexIndex source, std::span<const DigraphEdge> edges, bool begin);
        [[nodiscard]] reference operator*() const;
        const_iterator& operator++();
        const_iterator operator++(int);
        [[nodiscard]] bool operator==(const const_iterator& other) const;
        [[nodiscard]] bool operator!=(const const_iterator& other) const;
    };

    [[nodiscard]] const_iterator begin() const;
    [[nodiscard]] const_iterator end() const;
};

static_assert(std::ranges::forward_range<DigraphForwardEdgeIterator>);

class DigraphBackwardEdgeIterator
{
private:
    DigraphVertexIndex m_target;
    std::span<const DigraphEdge> m_edges;

public:
    DigraphBackwardEdgeIterator(DigraphVertexIndex target, std::span<const DigraphEdge> edges);

    class const_iterator
    {
    private:
        DigraphVertexIndex m_target;
        size_t m_pos;
        std::span<const DigraphEdge> m_edges;

        void advance();

    public:
        using difference_type = int;
        using value_type = const DigraphEdge;
        using pointer = value_type*;
        using reference = value_type&;
        using iterator_category = std::forward_iterator_tag;

        const_iterator();
        const_iterator(DigraphVertexIndex target, std::span<const DigraphEdge> edges, bool begin);
        [[nodiscard]] reference operator*() const;
        const_iterator& operator++();
        const_iterator operator++(int);
        [[nodiscard]] bool operator==(const const_iterator& other) const;
        [[nodiscard]] bool operator!=(const const_iterator& other) const;
    };

    [[nodiscard]] const_iterator begin() const;
    [[nodiscard]] const_iterator end() const;
};

class Digraph
{
public:
private:
    int m_num_vertices;
    bool m_is_directed;

    DigraphEdgeList m_edges;

public:
    explicit Digraph(bool is_directed = false);
    Digraph(int num_vertices, bool is_directed = false);

    /// @brief Add an edge to the graph and return the index to it.
    /// If the graph is undirected, then the backward edge has index + 1.
    DigraphEdgeIndex add_edge(DigraphVertexIndex source, DigraphVertexIndex target, DigraphEdgeWeight weight = 1.);

    /// @brief Reinitialize the graph to an empty graph with num_vertices many vertices.
    void reset(int num_vertices, bool is_directed = false);

    /**
     * Iterators
     */

    DigraphTargetIndexIterator get_targets(DigraphVertexIndex source) const;
    DigraphSourceIndexIterator get_sources(DigraphVertexIndex target) const;
    DigraphForwardEdgeIndexIterator get_forward_edge_indices(DigraphVertexIndex source) const;
    DigraphBackwardEdgeIndexIterator get_backward_edge_indices(DigraphVertexIndex target) const;
    DigraphForwardEdgeIterator get_forward_edges(DigraphVertexIndex source) const;
    DigraphBackwardEdgeIterator get_backward_edges(DigraphVertexIndex target) const;

    /**
     * Getters
     */
    int get_num_vertices() const;
    int get_num_edges() const;
    bool is_directed() const;
    const DigraphEdgeList& get_edges() const;
};

}
#endif
