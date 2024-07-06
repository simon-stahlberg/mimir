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

#include "nauty_dense_impl.hpp"

#include <cassert>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace nauty_wrapper
{

void DenseGraphImpl::allocate_graph(graph** out_graph)
{
    if (n_ > 0)
    {
        *out_graph = new graph[m_ * n_];
        EMPTYGRAPH0(*out_graph, m_, n_);
    }
}

void DenseGraphImpl::deallocate_graph(graph* the_graph)
{
    if (c_ > 0)
    {
        delete[] the_graph;
        the_graph = nullptr;
    }
}

DenseGraphImpl::DenseGraphImpl(int num_vertices, bool is_directed) :
    n_(num_vertices),
    c_(num_vertices),
    m_(SETWORDSNEEDED(n_)),
    is_directed_(is_directed),
    obtained_certificate_(false),
    graph_(nullptr),
    canon_graph_(nullptr)
{
    allocate_graph(&graph_);
    allocate_graph(&canon_graph_);
}

DenseGraphImpl::DenseGraphImpl(const DenseGraphImpl& other) : n_(other.n_), m_(other.m_), graph_(nullptr), canon_graph_(nullptr)
{
    allocate_graph(&graph_);
    allocate_graph(&canon_graph_);
    std::copy(other.graph_, other.graph_ + m_ * n_, graph_);
    std::copy(other.canon_graph_, other.canon_graph_ + m_ * n_, canon_graph_);
}

DenseGraphImpl& DenseGraphImpl::operator=(const DenseGraphImpl& other)
{
    if (this != &other)
    {
        deallocate_graph(graph_);
        deallocate_graph(canon_graph_);

        n_ = other.n_;
        c_ = other.c_;
        m_ = other.m_;
        is_directed_ = other.is_directed_;
        obtained_certificate_ = other.obtained_certificate_;

        allocate_graph(&graph_);
        allocate_graph(&canon_graph_);

        std::copy(other.graph_, other.graph_ + m_ * n_, graph_);
        std::copy(other.canon_graph_, other.canon_graph_ + m_ * n_, canon_graph_);
    }
    return *this;
}

DenseGraphImpl::DenseGraphImpl(DenseGraphImpl&& other) noexcept :
    n_(other.n_),
    c_(other.c_),
    m_(other.m_),
    is_directed_(other.is_directed_),
    obtained_certificate_(other.obtained_certificate_),
    graph_(other.graph_)
{
    other.graph_ = nullptr;
    other.canon_graph_ = nullptr;
}

DenseGraphImpl& DenseGraphImpl::operator=(DenseGraphImpl&& other) noexcept
{
    if (this != &other)
    {
        deallocate_graph(graph_);
        deallocate_graph(canon_graph_);

        n_ = other.n_;
        c_ = other.c_;
        m_ = other.m_;
        is_directed_ = other.is_directed_;
        obtained_certificate_ = other.obtained_certificate_;
        graph_ = other.graph_;
        canon_graph_ = other.canon_graph_;
        other.graph_ = nullptr;
        other.canon_graph_ = nullptr;
    }
    return *this;
}

DenseGraphImpl::~DenseGraphImpl()
{
    deallocate_graph(graph_);
    deallocate_graph(canon_graph_);
}

void DenseGraphImpl::add_edge(int source, int target)
{
    if (source >= n_ || target >= n_ || source < 0 || target < 0)
    {
        throw std::out_of_range("DenseGraphImpl::add_edge: Source or target vertex out of range.");
    }
    if (!is_directed_ && source == target)
    {
        throw std::logic_error("DenseGraphImpl::add_edge: Nauty does not support loops on undirected graphs.");
    }

    // Silently skip adding parallel edges because edges are unlabelled, and hence,
    // parallel edges do not encode additional information on the graph.

    // It is unnecessary to add antiparrallel edge in sparse representation
    // because nauty automatically takes care of this.
    ADDONEARC0(graph_, source, target, m_);
}

std::string DenseGraphImpl::compute_certificate(const mimir::Partitioning& partitioning)
{
    if (obtained_certificate_)
    {
        throw std::runtime_error(
            "DenseGraphImpl::compute_certificate: Tried to compute certificate twice for the same graph. We consider this a bug on the user side.");
    }
    if (static_cast<int>(partitioning.get_vertex_index_permutation().size()) != n_ || static_cast<int>(partitioning.get_partitioning().size()) != n_)
    {
        throw std::out_of_range("DenseGraphImpl::compute_certificate: The arrays lab or ptn are incompatible with number of vertices in the graph.");
    }

    int lab[n_], ptn[n_], orbits[n_];
    std::copy(partitioning.get_vertex_index_permutation().begin(), partitioning.get_vertex_index_permutation().end(), lab);
    std::copy(partitioning.get_partitioning().begin(), partitioning.get_partitioning().end(), ptn);

    static DEFAULTOPTIONS_GRAPH(options);
    options.defaultptn = FALSE;
    options.getcanon = TRUE;
    options.digraph = is_directed_;
    options.writeautoms = FALSE;
    statsblk stats;

    densenauty(graph_, lab, ptn, orbits, &options, &stats, m_, n_, canon_graph_);

    std::ostringstream oss;
    for (int i = 0; i < n_ * m_; ++i)
    {
        oss << canon_graph_[i] << " ";
    }

    obtained_certificate_ = true;

    return oss.str();
}

void DenseGraphImpl::reset(int num_vertices, bool is_directed)
{
    is_directed_ = is_directed;
    obtained_certificate_ = false;

    if (num_vertices > c_)
    {
        // Reallocate memory.
        deallocate_graph(graph_);
        deallocate_graph(canon_graph_);

        n_ = num_vertices;
        m_ = SETWORDSNEEDED(num_vertices);
        c_ = num_vertices;

        allocate_graph(&graph_);
        allocate_graph(&canon_graph_);
    }
    else
    {
        n_ = num_vertices;
        m_ = SETWORDSNEEDED(num_vertices);
        EMPTYGRAPH0(graph_, m_, n_);
        EMPTYGRAPH0(canon_graph_, m_, n_);
    }
}

bool DenseGraphImpl::is_directed() const { return is_directed_; }
}
