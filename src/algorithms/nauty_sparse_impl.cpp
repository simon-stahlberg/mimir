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

#include "nauty_utils.hpp"

#include <algorithm>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace nauty_wrapper
{

void SparseGraphImpl::copy_graph_data(const sparsegraph& in_graph, sparsegraph& out_graph) const
{
    std::copy(in_graph.v, in_graph.v + n_, out_graph.v);
    std::copy(in_graph.d, in_graph.d + n_, out_graph.d);
    std::copy(in_graph.e, in_graph.e + in_graph.nde, out_graph.e);
    out_graph.nde = in_graph.nde;
}

void SparseGraphImpl::initialize_graph_data(sparsegraph& out_graph) const
{
    out_graph.nv = n_;
    out_graph.nde = 0;  // No edges yet
    std::fill(out_graph.d, out_graph.d + n_, 0);
    for (size_t i = 0; i < n_; ++i)
    {
        out_graph.v[i] = i * n_;
    }
    std::fill(out_graph.e, out_graph.e + n_ * n_, 0);
}

void SparseGraphImpl::allocate_graph(sparsegraph& out_graph) const
{
    if (n_ > 0)
    {
        // Allocate the sparse graph structure, assuming a complete graph
        SG_INIT(out_graph);
        SG_ALLOC(out_graph, n_, n_ * n_, "malloc");
    }
    else
    {
        // Initialize lengths to zero when n is zero
        out_graph.vlen = 0;
        out_graph.dlen = 0;
        out_graph.elen = 0;
    }

    initialize_graph_data(out_graph);
}

void SparseGraphImpl::deallocate_graph(sparsegraph& the_graph) const
{
    if (c_ > 0)
    {
        // Deallocate the sparse graph structure
        SG_FREE(the_graph);
    }
}

SparseGraphImpl::SparseGraphImpl(size_t num_vertices) :
    n_(num_vertices),
    c_(n_),
    adj_matrix_(c_ * c_, false),
    use_default_ptn_(true),
    canon_coloring_(n_, 0),
    lab_(c_),
    ptn_(c_),
    canon_graph_repr_(),
    canon_graph_compressed_repr_()
{
    allocate_graph(graph_);
    allocate_graph(canon_graph_);
}

SparseGraphImpl::SparseGraphImpl(const SparseGraphImpl& other) :
    n_(other.n_),
    c_(other.c_),
    adj_matrix_(other.adj_matrix_),
    use_default_ptn_(other.use_default_ptn_),
    canon_coloring_(other.n_, 0),
    lab_(other.lab_),
    ptn_(other.ptn_),
    canon_graph_repr_(),
    canon_graph_compressed_repr_()
{
    canon_graph_repr_.str(other.canon_graph_repr_.str());
    canon_graph_compressed_repr_.str(other.canon_graph_compressed_repr_.str());

    allocate_graph(graph_);
    allocate_graph(canon_graph_);

    copy_graph_data(other.graph_, graph_);
    copy_graph_data(other.canon_graph_, canon_graph_);
}

SparseGraphImpl& SparseGraphImpl::operator=(const SparseGraphImpl& other)
{
    if (this != &other)
    {
        deallocate_graph(graph_);
        deallocate_graph(canon_graph_);

        n_ = other.n_;
        c_ = other.c_;
        adj_matrix_ = other.adj_matrix_;
        use_default_ptn_ = other.use_default_ptn_;
        canon_coloring_ = other.canon_coloring_;
        lab_ = other.lab_;
        ptn_ = other.ptn_;
        canon_graph_repr_.str(other.canon_graph_repr_.str());
        canon_graph_compressed_repr_.str(other.canon_graph_compressed_repr_.str());

        allocate_graph(graph_);
        allocate_graph(canon_graph_);

        copy_graph_data(other.graph_, graph_);
        copy_graph_data(other.canon_graph_, canon_graph_);
    }
    return *this;
}

SparseGraphImpl::SparseGraphImpl(SparseGraphImpl&& other) noexcept :
    n_(other.n_),
    c_(other.c_),
    adj_matrix_(other.adj_matrix_),
    graph_(other.graph_),
    use_default_ptn_(other.use_default_ptn_),
    canon_coloring_(std::move(other.canon_coloring_)),
    lab_(std::move(other.lab_)),
    ptn_(std::move(other.ptn_)),
    canon_graph_(other.canon_graph_),
    canon_graph_repr_(std::move(other.canon_graph_repr_)),
    canon_graph_compressed_repr_(std::move(other.canon_graph_compressed_repr_))
{
    SG_INIT(other.graph_);
    SG_INIT(other.canon_graph_);
}

SparseGraphImpl& SparseGraphImpl::operator=(SparseGraphImpl&& other) noexcept

{
    if (this != &other)
    {
        deallocate_graph(graph_);
        deallocate_graph(canon_graph_);

        n_ = other.n_;
        c_ = other.c_;
        adj_matrix_ = other.adj_matrix_;
        use_default_ptn_ = other.use_default_ptn_;
        canon_coloring_ = std::move(other.canon_coloring_);
        lab_ = std::move(other.lab_);
        ptn_ = std::move(other.ptn_);
        canon_graph_repr_ = std::move(other.canon_graph_repr_);
        canon_graph_compressed_repr_ = std::move(other.canon_graph_compressed_repr_);

        graph_ = other.graph_;
        SG_INIT(other.graph_);

        canon_graph_ = other.canon_graph_;
        SG_INIT(other.canon_graph_);
    }
    return *this;
}

SparseGraphImpl::~SparseGraphImpl()
{
    deallocate_graph(graph_);
    deallocate_graph(canon_graph_);
}

void SparseGraphImpl::add_vertex_coloring(const mimir::ColorList& vertex_coloring)
{
    if (vertex_coloring.size() != n_)
    {
        throw std::out_of_range("SparseGraphImpl::add_vertex_coloring: The vertex coloring is incompatible with number of vertices in the graph.");
    }

    canon_coloring_ = vertex_coloring;
    std::sort(canon_coloring_.begin(), canon_coloring_.end());

    initialize_lab_and_ptr(vertex_coloring, lab_, ptn_);
    use_default_ptn_ = false;
}

void SparseGraphImpl::add_edge(size_t source, size_t target)
{
    if (source >= n_ || target >= n_)
    {
        throw std::out_of_range("SparseGraphImpl::add_edge: Source or target vertex out of range.");
    }

    if (!adj_matrix_.at(source * n_ + target))
    {
        adj_matrix_.at(source * n_ + target) = true;

        graph_.e[source * n_ + graph_.d[source]] = target;
        ++graph_.d[source];
        ++graph_.nde;
    }
}

Certificate SparseGraphImpl::compute_certificate()
{
    bool is_directed_ = is_directed();
    bool has_loop_ = has_loop();

    if (!is_directed_ && has_loop_)
    {
        throw std::logic_error("SparseGraphImpl::compute_certificate: Nauty does not support loops on undirected graphs.");
    }

    static DEFAULTOPTIONS_SPARSEGRAPH(options);
    options.defaultptn = use_default_ptn_;
    options.getcanon = TRUE;
    options.digraph = is_directed_;
    options.writeautoms = FALSE;

    int orbits[n_];

    statsblk stats;

    std::fill(canon_graph_.d, canon_graph_.d + n_, 0);
    std::fill(canon_graph_.v, canon_graph_.v + n_, 0);
    std::fill(canon_graph_.e, canon_graph_.e + n_ * n_, 0);

    sparsenauty(&graph_, lab_.data(), ptn_.data(), orbits, &options, &stats, &canon_graph_);

    // According to documentation:
    //   canon_graph has contiguous adjacency lists that are not necessarily sorted
    sortlists_sg(&canon_graph_);

    // Clear streams
    canon_graph_repr_.str(std::string());
    canon_graph_repr_.clear();
    canon_graph_compressed_repr_.str(std::string());
    canon_graph_compressed_repr_.clear();

    // Compute conon graph repr.
    canon_graph_repr_ << canon_graph_;

    // Compress canon graph repr.
    boost::iostreams::filtering_ostream boost_filtering_ostream;
    boost_filtering_ostream.push(boost::iostreams::gzip_compressor());
    boost_filtering_ostream.push(canon_graph_compressed_repr_);
    boost::iostreams::copy(canon_graph_repr_, boost_filtering_ostream);
    boost::iostreams::close(boost_filtering_ostream);
    // We usually see compression ratio ~ 2
    // std::cout << "Compression ratio: " << (double) canon_graph_repr_.str().size() / canon_graph_compressed_repr_.str().size() << std::endl;

    return Certificate(canon_graph_compressed_repr_.str(), canon_coloring_);
}

void SparseGraphImpl::clear(size_t num_vertices)
{
    use_default_ptn_ = true;

    if (num_vertices > c_)
    {
        /* Reallocate memory. */
        deallocate_graph(graph_);
        deallocate_graph(canon_graph_);

        n_ = num_vertices;
        c_ = num_vertices;
        canon_coloring_ = mimir::ColorList(n_, 0);
        lab_ = std::vector<int>(n_);
        ptn_ = std::vector<int>(n_);
        adj_matrix_ = std::vector<bool>(n_ * n_, false);

        allocate_graph(graph_);
        allocate_graph(canon_graph_);
    }
    else
    {
        /* Reuse memory. */
        n_ = num_vertices;
        std::fill(canon_coloring_.begin(), canon_coloring_.begin() + n_, 0);
        std::fill(adj_matrix_.begin(), adj_matrix_.begin() + n_ * n_, false);

        initialize_graph_data(graph_);
        initialize_graph_data(canon_graph_);
    }
}

bool SparseGraphImpl::is_directed() const
{
    for (size_t source = 0; source < n_; ++source)
    {
        for (size_t target = source + 1; target < n_; ++target)
        {
            if (adj_matrix_.at(source * n_ + target) != adj_matrix_.at(target * n_ + source))
            {
                return true;
            }
        }
    }
    return false;
}

bool SparseGraphImpl::has_loop() const
{
    for (size_t source = 0; source < n_; ++source)
    {
        if (adj_matrix_.at(source * n_ + source))
        {
            return true;
        }
    }
    return false;
}

std::ostream& operator<<(std::ostream& out, const sparsegraph& graph)
{
    // TODO: we could use log_2(nv) many bits to encode the adjacency lists
    // and log_2(nde) to encode number of transitions and degrees to save memory.
    out << "nv:" << graph.nv << "\n"
        << "nde:" << graph.nde << "\n"
        << "d:";
    for (int i = 0; i < graph.nv; ++i)
    {
        out << graph.d[i] << (i == graph.nv - 1 ? "\n" : ",");
    }
    out << "v:";
    for (int i = 0; i < graph.nv; ++i)
    {
        out << graph.v[i] << (i == graph.nv - 1 ? "\n" : ",");
    }
    out << "e:";
    for (size_t i = 0; i < graph.nde; ++i)
    {
        out << graph.e[i] << (i == graph.nde - 1 ? "\n" : ",");
    }

    return out;
}
}
