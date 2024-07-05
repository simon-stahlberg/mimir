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

#include "nauty_sparse_impl.hpp"

#include <iostream>
#include <sstream>
#include <stdexcept>

namespace nauty_wrapper
{

void SparseGraphImpl::allocate_graph(sparsegraph& out_graph)
{
    if (n_ > 0)
    {
        // Allocate the sparse graph structure, assuming a complete graph
        SG_INIT(out_graph);
        SG_ALLOC(out_graph, n_, 2 * n_ * n_, "malloc");

        out_graph.nv = n_;
        out_graph.nde = 0;  // No edges yet

        // Initialize arrays
        std::fill(out_graph.d, out_graph.d + n_, 0);
        std::fill(out_graph.v, out_graph.v + n_, 0);
        std::fill(out_graph.e, out_graph.e + 2 * n_ * n_, 0);
    }
    else
    {
        // Initialize lengths to zero when n is zero
        out_graph.vlen = 0;
        out_graph.dlen = 0;
        out_graph.elen = 0;
    }
}

void SparseGraphImpl::deallocate_graph(sparsegraph& the_graph)
{
    if (c_ > 0)
    {
        // Deallocate the sparse graph structure
        SG_FREE(the_graph);
    }
}

SparseGraphImpl::SparseGraphImpl(int num_vertices, bool is_directed) :
    n_(num_vertices),
    c_(num_vertices),
    is_directed_(is_directed),
    obtained_certificate_(false)
{
    allocate_graph(graph_);
    allocate_graph(canon_graph_);
}

SparseGraphImpl::SparseGraphImpl(const SparseGraphImpl& other)
{
    n_ = other.n_;
    c_ = other.c_;
    is_directed_ = other.is_directed_;
    obtained_certificate_ = other.obtained_certificate_;

    allocate_graph(graph_);
    allocate_graph(canon_graph_);

    std::copy(other.graph_.v, other.graph_.v + n_, graph_.v);
    std::copy(other.graph_.d, other.graph_.d + n_, graph_.d);
    std::copy(other.graph_.e, other.graph_.e + other.graph_.nde, graph_.e);
    graph_.nde = other.graph_.nde;

    std::copy(other.canon_graph_.v, other.canon_graph_.v + n_, canon_graph_.v);
    std::copy(other.canon_graph_.d, other.canon_graph_.d + n_, canon_graph_.d);
    std::copy(other.canon_graph_.e, other.canon_graph_.e + other.canon_graph_.nde, canon_graph_.e);
    canon_graph_.nde = other.canon_graph_.nde;
}

SparseGraphImpl& SparseGraphImpl::operator=(const SparseGraphImpl& other)
{
    if (this != &other)
    {
        deallocate_graph(graph_);
        deallocate_graph(canon_graph_);

        n_ = other.n_;
        c_ = other.c_;
        is_directed_ = other.is_directed_;
        obtained_certificate_ = other.obtained_certificate_;

        allocate_graph(graph_);
        allocate_graph(canon_graph_);

        std::copy(other.graph_.v, other.graph_.v + n_, graph_.v);
        std::copy(other.graph_.d, other.graph_.d + n_, graph_.d);
        std::copy(other.graph_.e, other.graph_.e + other.graph_.nde, graph_.e);
        graph_.nde = other.graph_.nde;

        std::copy(other.canon_graph_.v, other.canon_graph_.v + n_, canon_graph_.v);
        std::copy(other.canon_graph_.d, other.canon_graph_.d + n_, canon_graph_.d);
        std::copy(other.canon_graph_.e, other.canon_graph_.e + other.canon_graph_.nde, canon_graph_.e);
        canon_graph_.nde = other.canon_graph_.nde;
    }
    return *this;
}

SparseGraphImpl::SparseGraphImpl(SparseGraphImpl&& other) noexcept :
    n_(other.n_),
    c_(other.c_),
    is_directed_(other.is_directed_),
    obtained_certificate_(other.obtained_certificate_),
    graph_(other.graph_),
    canon_graph_(other.canon_graph_)
{
    other.graph_.v = nullptr;
    other.graph_.d = nullptr;
    other.graph_.e = nullptr;
    other.graph_.vlen = 0;
    other.graph_.dlen = 0;
    other.graph_.elen = 0;
    other.graph_.nde = 0;

    other.canon_graph_.v = nullptr;
    other.canon_graph_.d = nullptr;
    other.canon_graph_.e = nullptr;
    other.canon_graph_.vlen = 0;
    other.canon_graph_.dlen = 0;
    other.canon_graph_.elen = 0;
    other.canon_graph_.nde = 0;
}

SparseGraphImpl& SparseGraphImpl::operator=(SparseGraphImpl&& other) noexcept

{
    if (this != &other)
    {
        deallocate_graph(graph_);
        deallocate_graph(canon_graph_);

        n_ = other.n_;
        c_ = other.c_;
        is_directed_ = other.is_directed_;
        obtained_certificate_ = other.obtained_certificate_;

        graph_ = other.graph_;
        other.graph_.v = nullptr;
        other.graph_.d = nullptr;
        other.graph_.e = nullptr;
        other.graph_.vlen = 0;
        other.graph_.dlen = 0;
        other.graph_.elen = 0;
        other.graph_.nde = 0;

        canon_graph_ = other.canon_graph_;
        other.canon_graph_.v = nullptr;
        other.canon_graph_.d = nullptr;
        other.canon_graph_.e = nullptr;
        other.canon_graph_.vlen = 0;
        other.canon_graph_.dlen = 0;
        other.canon_graph_.elen = 0;
        other.canon_graph_.nde = 0;
    }
    return *this;
}

SparseGraphImpl::~SparseGraphImpl()
{
    deallocate_graph(graph_);
    deallocate_graph(canon_graph_);
}

void SparseGraphImpl::add_edge(int src, int dst)
{
    if (src >= n_ || dst >= n_ || src < 0 || dst < 0)
    {
        throw std::out_of_range("Source or destination vertex out of range.");
    }
    // TODO: this is wrong!
    graph_.e[graph_.nde++] = dst;
    graph_.d[src]++;
    if (!is_directed_)
    {
        graph_.e[graph_.nde++] = src;
        graph_.d[dst]++;
    }

    if (graph_.nde >= 2 * n_ * n_)
    {
        throw std::out_of_range("Not enough space for edges.");
    }
}

std::string SparseGraphImpl::compute_certificate(const mimir::Partitioning& partitioning)
{
    if (obtained_certificate_)
    {
        throw std::runtime_error("Tried to compute certificate twice for the same graph. That is most likely a bug on the user side.");
    }
    if (static_cast<int>(partitioning.get_vertex_index_permutation().size()) != n_ || static_cast<int>(partitioning.get_partitioning().size()) != n_)
    {
        throw std::out_of_range("lab or ptn is incompatible with number of vertices in the graph.");
    }

    int lab[n_], ptn[n_], orbits[n_];
    std::copy(partitioning.get_vertex_index_permutation().begin(), partitioning.get_vertex_index_permutation().end(), lab);
    std::copy(partitioning.get_partitioning().begin(), partitioning.get_partitioning().end(), ptn);

    static DEFAULTOPTIONS_SPARSEGRAPH(options);
    options.defaultptn = FALSE;
    options.getcanon = TRUE;
    options.digraph = is_directed_;
    options.writeautoms = FALSE;

    statsblk stats;

    // std::cout << "graph_.nv: " << graph_.nv << std::endl;
    // std::cout << "graph_.nde: " << graph_.nde << std::endl;
    // std::cout << "graph_.vlen: " << graph_.vlen << std::endl;
    // std::cout << "graph_.dlen: " << graph_.dlen << std::endl;
    // std::cout << "graph_.elen: " << graph_.elen << std::endl;

    // std::cout << "canon_graph_.nv: " << canon_graph_.nv << std::endl;
    // std::cout << "canon_graph_.nde: " << canon_graph_.nde << std::endl;
    // std::cout << "canon_graph_.vlen: " << canon_graph_.vlen << std::endl;
    // std::cout << "canon_graph_.dlen: " << canon_graph_.dlen << std::endl;
    // std::cout << "canon_graph_.elen: " << canon_graph_.elen << std::endl;

    sparsenauty(&graph_, lab, ptn, orbits, &options, &stats, &canon_graph_);

    std::ostringstream oss;
    for (size_t i = 0; i < canon_graph_.nde; ++i)
    {
        oss << canon_graph_.e[i] << " ";
    }

    obtained_certificate_ = true;

    return oss.str();
}

void SparseGraphImpl::reset(int num_vertices, bool is_directed)
{
    is_directed_ = is_directed;
    obtained_certificate_ = false;

    if (num_vertices > c_)
    {
        // Reallocate memory.
        deallocate_graph(graph_);
        deallocate_graph(canon_graph_);

        n_ = num_vertices;
        c_ = num_vertices;

        allocate_graph(graph_);
        allocate_graph(canon_graph_);
    }
    else
    {
        // Reuse memory.
        n_ = num_vertices;

        graph_.nv = n_;
        graph_.nde = 0;
        std::fill(graph_.d, graph_.d + n_, 0);
        std::fill(graph_.v, graph_.v + n_, 0);
        std::fill(graph_.e, graph_.e + 2 * n_ * n_, 0);

        canon_graph_.nv = n_;
        canon_graph_.nde = 0;
        std::fill(canon_graph_.d, canon_graph_.d + n_, 0);
        std::fill(canon_graph_.v, canon_graph_.v + n_, 0);
        std::fill(canon_graph_.e, canon_graph_.e + 2 * n_ * n_, 0);
    }
}

bool SparseGraphImpl::is_directed() const { return is_directed_; }

}
