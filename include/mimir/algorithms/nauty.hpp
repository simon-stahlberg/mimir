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

#include "mimir/common/printers.hpp"
#include "mimir/graphs/declarations.hpp"
#include "mimir/graphs/digraph_vertex_colored.hpp"

#include <memory>
#include <ostream>
#include <vector>

/// @brief Wrap a namespace around nauty's interface
namespace nauty_wrapper
{
class DenseGraphImpl;
class SparseGraphImpl;

/// @brief `Certificate` encapsulates a canonical graph representation of a vertex-colored graph.
class Certificate
{
private:
    /* Core members for comparing certificates. */
    std::string m_canonical_graph;
    // Since the canonical graph is computed using a vertex partitioning, we additionally require a canonical representation of the vertex colors to distinguish
    // graphs with different canonical vertex coloring but identical vertex partitioning.
    mimir::ColorList m_canonical_coloring;

public:
    Certificate(std::string canonical_graph, mimir::ColorList canonical_coloring);

    const std::string& get_canonical_graph() const;
    const mimir::ColorList& get_canonical_coloring() const;
};

extern bool operator==(const Certificate& lhs, const Certificate& rhs);
extern bool operator<(const Certificate& lhs, const Certificate& rhs);

extern std::ostream& operator<<(std::ostream& os, const Certificate& element);

struct UniqueCertificateSharedPtrHash
{
    size_t operator()(const std::shared_ptr<const Certificate>& element) const;
};

struct UniqueCertificateSharedPtrEqualTo
{
    size_t operator()(const std::shared_ptr<const Certificate>& lhs, const std::shared_ptr<const Certificate>& rhs) const;
};

/// @brief `DenseGraph` encapsulates a dense graph representation compatible with Nauty.
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
    explicit DenseGraph(const mimir::StaticVertexColoredDigraph& digraph);

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

    /// @brief Compute a graph certificate.
    Certificate compute_certificate() const;

    /// @brief Clear the graph data structures by changing the number of vertices and removing all edges.
    /// @param num_vertices is the new number of vertices.
    void clear(size_t num_vertices);

    /// @brief Return true iff the graph is directed.
    bool is_directed() const;
};

/// @brief `SparseGraph` encapsulates a sparse graph representation compatible with Nauty.
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
    explicit SparseGraph(const mimir::StaticVertexColoredDigraph& digraph);

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

    /// @brief Compute a graph certificate.
    Certificate compute_certificate();

    /// @brief Clear the graph data structures by changing the number of vertices and removing all edges.
    /// @param num_vertices is the new number of vertices.
    void clear(size_t num_vertices);

    /// @brief Return true iff the graph is directed.
    bool is_directed() const;
};

}

template<>
struct std::hash<nauty_wrapper::Certificate>
{
    size_t operator()(const nauty_wrapper::Certificate& element) const;
};

#endif
