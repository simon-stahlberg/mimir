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

#ifndef MIMIR_GRAPHS_ALGORITHMS_NAUTY_HPP_
#define MIMIR_GRAPHS_ALGORITHMS_NAUTY_HPP_

#include "mimir/common/printers.hpp"
#include "mimir/graphs/concrete/digraph_vertex_colored.hpp"
#include "mimir/graphs/declarations.hpp"
#include "mimir/graphs/graph_interface.hpp"

#include <loki/details/utils/equal_to.hpp>
#include <loki/details/utils/hash.hpp>
#include <memory>
#include <ostream>
#include <vector>

/// @brief Wrap a namespace around nauty's interface
namespace mimir::graphs::nauty
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
    mimir::graphs::ColorList m_canonical_coloring;

public:
    Certificate(std::string canonical_graph, mimir::graphs::ColorList canonical_coloring);
    Certificate(const Certificate& other) = delete;
    Certificate& operator=(const Certificate& other) = delete;
    Certificate(Certificate&& other) = default;
    Certificate& operator=(Certificate&& other) = default;

    const std::string& get_canonical_graph() const;
    const mimir::graphs::ColorList& get_canonical_coloring() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const { return std::tuple(std::cref(get_canonical_graph()), std::cref(get_canonical_coloring())); }
};

extern bool operator==(const Certificate& lhs, const Certificate& rhs);
extern bool operator<(const Certificate& lhs, const Certificate& rhs);

extern std::ostream& operator<<(std::ostream& os, const Certificate& element);

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

    template<mimir::graphs::IsVertexColoredGraph Graph>
    explicit DenseGraph(const Graph& graph) : DenseGraph(graph.get_num_vertices())
    {
        /* Remap indices to 0,1,... indexing schema. */
        auto remap = mimir::IndexMap<mimir::Index>();
        for (const auto& vertex : graph.get_vertices())
        {
            remap.emplace(vertex.get_index(), remap.size());
        }

        /* Add edges. */
        for (const auto& edge : graph.get_edges())
        {
            add_edge(remap.at(edge.get_source()), remap.at(edge.get_target()));
        }

        /* Add vertex coloring. */
        auto coloring = mimir::graphs::ColorList(graph.get_num_vertices());
        for (const auto& vertex : graph.get_vertices())
        {
            coloring.at(remap.at(vertex.get_index())) = get_color(vertex);
        }
        add_vertex_coloring(coloring);
    }

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
    void add_vertex_coloring(const mimir::graphs::ColorList& vertex_coloring);

    /// @brief Add a directed edge to the graph.
    ///
    /// Throws an exception if `source` or `target` is out of bounds.
    /// @param source is the index of the source vertex.
    /// @param target is the index of the target vertex.
    void add_edge(size_t source, size_t target);

    /// @brief Clear the graph data structures by changing the number of vertices and removing all edges.
    /// @param num_vertices is the new number of vertices.
    void clear(size_t num_vertices);

    /// @brief Return true iff the graph is directed.
    bool is_directed() const;

    friend Certificate compute_certificate(const DenseGraph& graph);
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

    template<mimir::graphs::IsVertexColoredGraph Graph>
    explicit SparseGraph(const Graph& graph) : SparseGraph(graph.get_num_vertices())
    {
        /* Remap indices to 0,1,... indexing schema. */
        auto remap = mimir::IndexMap<mimir::Index>();
        for (const auto& vertex : graph.get_vertices())
        {
            remap.emplace(vertex.get_index(), remap.size());
        }

        /* Add edges. */
        for (const auto& edge : graph.get_edges())
        {
            add_edge(remap.at(edge.get_source()), remap.at(edge.get_target()));
        }

        /* Add vertex coloring. */
        auto coloring = mimir::graphs::ColorList(graph.get_num_vertices());
        for (const auto& vertex : graph.get_vertices())
        {
            coloring.at(remap.at(vertex.get_index())) = get_color(vertex);
        }
        add_vertex_coloring(coloring);
    }

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
    void add_vertex_coloring(const mimir::graphs::ColorList& vertex_coloring);

    /// @brief Add a directed edge to the graph.
    ///
    /// Throws an exception if `source` or `target` is out of bounds.
    /// @param source is the index of the source vertex.
    /// @param target is the index of the target vertex.
    void add_edge(size_t source, size_t target);

    /// @brief Clear the graph data structures by changing the number of vertices and removing all edges.
    /// @param num_vertices is the new number of vertices.
    void clear(size_t num_vertices);

    /// @brief Return true iff the graph is directed.
    bool is_directed() const;

    friend Certificate compute_certificate(const SparseGraph& graph);
};

/// @brief Compute the nauty graph `Certificate` for a given graph that satisfies the `mimir::graphs::IsVertexColoredGraph` graph concept.
/// This function internally uses the `SparseGraph` representation for simplicity.
/// To use `DenseGraph`, one has to construct it and call `compute_certificate`.
/// @tparam is the type of the graph.
/// @param graph is the graph that satisfies the `mimir::graphs::IsVertexColoredGraph` graph concept.
/// @return is the compressed nauty graph `Certificate`.
template<mimir::graphs::IsVertexColoredGraph Graph>
Certificate compute_certificate(const Graph& graph)
{
    return compute_certificate(SparseGraph(graph));
}

}

#endif
