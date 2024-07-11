/*
 * Copyright (C) 2023 Simon Stahlberg
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

#ifndef MIMIR_ALGORITHMS_NAUTY_HPP_
#define MIMIR_ALGORITHMS_NAUTY_HPP_

#include "mimir/graphs/color.hpp"
#include "mimir/graphs/digraph_vertex_colored.hpp"

#include <memory>
#include <vector>

/// @brief Wrap a namespace around nauty's interface
namespace nauty_wrapper
{
class DenseGraphImpl;
class SparseGraphImpl;

class DenseGraph
{
private:
    std::unique_ptr<DenseGraphImpl> m_impl;

public:
    /// @brief Create an empty `DenseGraph` with zero vertices.
    DenseGraph();

    /// @brief Create a `DenseGraph` with `num_vertices` many vertices and zero edges.
    /// @param num_vertices is the number of vertices in the graph.
    explicit DenseGraph(size_t num_vertices);

    /// @brief Create a `DenseGraph` from a vertex colored `digraph`.
    /// @param digraph is the vertex colored digraph.
    explicit DenseGraph(const mimir::VertexColoredDigraph& digraph);

    DenseGraph(const DenseGraph& other);
    DenseGraph& operator=(const DenseGraph& other);
    DenseGraph(DenseGraph&& other) noexcept;
    DenseGraph& operator=(DenseGraph&& other) noexcept;
    ~DenseGraph();

    /// @brief Add colors to the vertices of the graph.
    ///
    /// Throws an exception if the size of the `vertex_coloring`
    /// is not equal to the number of vertices in the graph.
    /// @param vertex_coloring are the colors of the vertices.
    void add_vertex_coloring(const mimir::ColorList& vertex_coloring);

    /// @brief Add a directed edge to the graph.
    ///
    /// Throws an exception if `source` or `target` is out of bounds.
    /// @param source is the index of the source vertex.
    /// @param target is the index of the target vertex.
    void add_edge(size_t source, size_t target);

    /// @brief Compute a compressed string representation of the canonical graph of the graph.
    std::string compute_certificate() const;

    /// @brief Reinitialize the graph by changing the number of vertices and removing all edges.
    /// @param num_vertices is the new number of vertices.
    void reset(size_t num_vertices);

    /// @brief Return true iff the graph is directed.
    bool is_directed() const;
};

class SparseGraph
{
private:
    std::unique_ptr<SparseGraphImpl> m_impl;

public:
    /// @brief Create an empty `SparseGraph` with zero vertices.
    SparseGraph();

    /// @brief Create `SparseGraph` with `num_vertices` many vertices and zero edges.
    /// @param num_vertices is the number of vertices in the graph.
    explicit SparseGraph(size_t num_vertices);

    /// @brief Create a `SparseGraph` from a vertex colored `digraph`.
    /// @param digraph is the vertex colored digraph.
    explicit SparseGraph(const mimir::VertexColoredDigraph& digraph);

    SparseGraph(const SparseGraph& other);
    SparseGraph& operator=(const SparseGraph& other);
    SparseGraph(SparseGraph&& other) noexcept;
    SparseGraph& operator=(SparseGraph&& other) noexcept;
    ~SparseGraph();

    /// @brief Add colors to the vertices of the graph.
    ///
    /// Throws an exception if the size of the `vertex_coloring`
    /// is not equal to the number of vertices in the graph.
    /// @param vertex_coloring are the colors of the vertices.
    void add_vertex_coloring(const mimir::ColorList& vertex_coloring);

    /// @brief Add a directed edge to the graph.
    ///
    /// Throws an exception if `source` or `target` is out of bounds.
    /// @param source is the index of the source vertex.
    /// @param target is the index of the target vertex.
    void add_edge(size_t source, size_t target);

    /// @brief Compute a compressed string representation of the canonical graph of the graph.
    std::string compute_certificate();

    /// @brief Reinitialize the graph by changing the number of vertices and removing all edges.
    /// @param num_vertices is the new number of vertices.
    void reset(size_t num_vertices);

    /// @brief Return true iff the graph is directed.
    bool is_directed() const;
};

}

#endif
