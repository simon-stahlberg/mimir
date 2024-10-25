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

#include "nauty_utils.hpp"

#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filtering_stream.hpp>
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

DenseGraphImpl::DenseGraphImpl(size_t num_vertices) :
    n_(num_vertices),
    c_(n_),
    m_(SETWORDSNEEDED(n_)),
    graph_(nullptr),
    use_default_ptn_(true),
    canon_coloring_(n_, 0),
    lab_(n_),
    ptn_(n_),
    canon_graph_(nullptr)
{
    allocate_graph(&graph_);
    allocate_graph(&canon_graph_);
}

DenseGraphImpl::DenseGraphImpl(const DenseGraphImpl& other) :
    n_(other.n_),
    c_(other.c_),
    m_(other.m_),
    graph_(nullptr),
    use_default_ptn_(other.use_default_ptn_),
    canon_coloring_(other.canon_coloring_),
    lab_(other.lab_),
    ptn_(other.ptn_),
    canon_graph_(nullptr),
    canon_graph_repr_(),
    canon_graph_compressed_repr_()
{
    canon_graph_repr_.str(other.canon_graph_repr_.str());
    canon_graph_compressed_repr_.str(other.canon_graph_compressed_repr_.str());

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
        use_default_ptn_ = other.use_default_ptn_;
        canon_coloring_ = other.canon_coloring_;
        lab_ = other.lab_;
        ptn_ = other.ptn_;
        canon_graph_repr_.str(other.canon_graph_repr_.str());
        canon_graph_compressed_repr_.str(other.canon_graph_compressed_repr_.str());

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
    graph_(other.graph_),
    use_default_ptn_(other.use_default_ptn_),
    canon_coloring_(std::move(other.canon_coloring_)),
    lab_(std::move(other.lab_)),
    ptn_(std::move(other.ptn_)),
    canon_graph_(other.canon_graph_),
    canon_graph_repr_(std::move(other.canon_graph_repr_)),
    canon_graph_compressed_repr_(std::move(other.canon_graph_compressed_repr_))
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
        graph_ = other.graph_;
        use_default_ptn_ = other.use_default_ptn_;
        canon_coloring_ = std::move(other.canon_coloring_);
        lab_ = std::move(other.lab_);
        ptn_ = std::move(other.ptn_);
        canon_graph_ = other.canon_graph_;
        canon_graph_repr_ = std::move(other.canon_graph_repr_);
        canon_graph_compressed_repr_ = std::move(other.canon_graph_compressed_repr_);

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

void DenseGraphImpl::add_vertex_coloring(const mimir::ColorList& vertex_coloring)
{
    if (vertex_coloring.size() != n_)
    {
        throw std::out_of_range("DenseGraphImpl::add_vertex_coloring: The vertex coloring is incompatible with number of vertices in the graph.");
    }

    canon_coloring_ = vertex_coloring;
    std::sort(canon_coloring_.begin(), canon_coloring_.end());

    initialize_lab_and_ptr(vertex_coloring, lab_, ptn_);
    use_default_ptn_ = false;
}

void DenseGraphImpl::add_edge(size_t source, size_t target)
{
    if (source >= n_ || target >= n_)
    {
        throw std::out_of_range("DenseGraphImpl::add_edge: Source or target vertex out of range.");
    }
    ADDONEARC0(graph_, source, target, m_);
}

Certificate DenseGraphImpl::compute_certificate()
{
    const auto is_directed_ = is_directed();
    const auto has_loop_ = has_loop();

    if (!is_directed_ && has_loop_)
    {
        throw std::logic_error("DenseGraphImpl::compute_certificate: Nauty does not support loops on undirected graphs.");
    }

    static DEFAULTOPTIONS_GRAPH(options);
    options.defaultptn = FALSE;
    options.getcanon = TRUE;
    options.digraph = is_directed_;
    options.writeautoms = FALSE;

    int orbits[n_];

    statsblk stats;

    densenauty(graph_, lab_.data(), ptn_.data(), orbits, &options, &stats, m_, n_, canon_graph_);

    // Clear streams
    canon_graph_repr_.str(std::string());
    canon_graph_repr_.clear();
    canon_graph_compressed_repr_.str(std::string());
    canon_graph_compressed_repr_.clear();

    // Compute conon graph repr.
    for (size_t i = 0; i < n_ * m_; ++i)
    {
        canon_graph_repr_ << canon_graph_[i] << " ";
    }

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

void DenseGraphImpl::clear(size_t num_vertices)
{
    use_default_ptn_ = true;

    if (num_vertices > c_)
    {
        // Reallocate memory.
        deallocate_graph(graph_);
        deallocate_graph(canon_graph_);

        n_ = num_vertices;
        m_ = SETWORDSNEEDED(num_vertices);
        c_ = num_vertices;
        canon_coloring_ = mimir::ColorList(n_, 0);
        lab_ = std::vector<int>(n_);
        ptn_ = std::vector<int>(n_);

        allocate_graph(&graph_);
        allocate_graph(&canon_graph_);
    }
    else
    {
        n_ = num_vertices;
        m_ = SETWORDSNEEDED(num_vertices);

        std::fill(canon_coloring_.begin(), canon_coloring_.begin() + n_, 0);

        EMPTYGRAPH0(graph_, m_, n_);
        EMPTYGRAPH0(canon_graph_, m_, n_);
    }
}

bool DenseGraphImpl::is_directed() const
{
    for (size_t source = 0; source < n_; ++source)
    {
        set* source_row = GRAPHROW1(graph_, source, m_);
        for (size_t target = source + 1; target < n_; ++target)
        {
            set* target_row = GRAPHROW1(graph_, target, m_);
            if (ISELEMENT1(source_row, target) != ISELEMENT1(target_row, source))
            {
                return true;
            }
        }
    }
    return false;
}

bool DenseGraphImpl::has_loop() const
{
    for (size_t source = 0; source < n_; ++source)
    {
        set* source_row = GRAPHROW1(graph_, source, m_);
        if (ISELEMENT1(source_row, source))
        {
            return true;
        }
    }
    return false;
}
}
