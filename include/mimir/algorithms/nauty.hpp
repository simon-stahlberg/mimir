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

#include "mimir/graphs/digraph.hpp"
#include "mimir/graphs/partitioning.hpp"

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
    DenseGraph();
    explicit DenseGraph(size_t num_vertices);
    DenseGraph(const DenseGraph& other);
    DenseGraph& operator=(const DenseGraph& other);
    DenseGraph(DenseGraph&& other) noexcept;
    DenseGraph& operator=(DenseGraph&& other) noexcept;
    ~DenseGraph();

    void add_edge(size_t source, size_t target);

    /// @brief Compute a certificate for the graph and the given vertex partitioning.
    std::string compute_certificate(const mimir::Partitioning& partitioning) const;

    /// @brief Reinitialize the graph.
    void reset(size_t num_vertices);

    bool is_directed() const;
};

class DenseGraphFactory
{
private:
    DenseGraph m_graph;

public:
    DenseGraph& create_from_digraph(const mimir::graphs::Digraph& digraph);
};

class SparseGraph
{
private:
    std::unique_ptr<SparseGraphImpl> m_impl;

public:
    SparseGraph();
    SparseGraph(size_t num_vertices);
    SparseGraph(const SparseGraph& other);
    SparseGraph& operator=(const SparseGraph& other);
    SparseGraph(SparseGraph&& other) noexcept;
    SparseGraph& operator=(SparseGraph&& other) noexcept;
    ~SparseGraph();

    void add_edge(size_t source, size_t target);

    /// @brief Compute a certificate for the graph and the given vertex partitioning.
    std::string compute_certificate(const mimir::Partitioning& partitioning);

    /// @brief Reinitialize the graph.
    void reset(size_t num_vertices);

    bool is_directed() const;
};

class SparseGraphFactory
{
private:
    SparseGraph m_graph;

public:
    SparseGraph& create_from_digraph(const mimir::graphs::Digraph& digraph);
};

}

#endif
