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

#include "mimir/algorithms/nauty.hpp"

#include "mimir/common/hash.hpp"
#include "mimir/common/types.hpp"
#include "nauty_dense_impl.hpp"
#include "nauty_sparse_impl.hpp"

#include <cassert>
#include <iostream>
#include <stdexcept>

namespace nauty_wrapper
{
using mimir::operator<<;

/* Certificate */
Certificate::Certificate(std::string nauty_certificate, mimir::ColorList canonical_initial_coloring) :
    m_canonical_graph(std::move(nauty_certificate)),
    m_canonical_coloring(std::move(canonical_initial_coloring))
{
    assert(std::is_sorted(m_canonical_coloring.begin(), m_canonical_coloring.end()));
}

const std::string& Certificate::get_canonical_graph() const { return m_canonical_graph; }

const mimir::ColorList& Certificate::get_canonical_coloring() const { return m_canonical_coloring; }

size_t UniqueCertificateSharedPtrHash::operator()(const std::shared_ptr<const Certificate>& element) const { return std::hash<Certificate>()(*element); }

size_t UniqueCertificateSharedPtrEqualTo::operator()(const std::shared_ptr<const Certificate>& lhs, const std::shared_ptr<const Certificate>& rhs) const
{
    return *lhs == *rhs;
}

bool operator==(const Certificate& lhs, const Certificate& rhs)
{
    if (&lhs != &rhs)
    {
        return (lhs.get_canonical_coloring() == rhs.get_canonical_coloring()) && (lhs.get_canonical_graph() == rhs.get_canonical_graph());
    }
    return true;
}

bool operator<(const Certificate& lhs, const Certificate& rhs)
{
    if (&lhs != &rhs)
    {
        if (lhs.get_canonical_coloring() == rhs.get_canonical_coloring())
        {
            return lhs.get_canonical_graph() < rhs.get_canonical_graph();
        }
        return lhs.get_canonical_coloring() < rhs.get_canonical_coloring();
    }
    return false;
}

std::ostream& operator<<(std::ostream& os, const Certificate& element)
{
    os << "CertificateNauty("
       << "canonical_graph=" << element.get_canonical_graph() << ", "
       << "canonical_coloring=" << element.get_canonical_coloring() << ")";
    return os;
}

/* Graph */
DenseGraph::DenseGraph() : m_impl(std::make_unique<DenseGraphImpl>(0)) {}

DenseGraph::DenseGraph(size_t num_vertices) : m_impl(std::make_unique<DenseGraphImpl>(num_vertices)) {}

DenseGraph::DenseGraph(const mimir::StaticVertexColoredDigraph& digraph) : m_impl(std::make_unique<DenseGraphImpl>(digraph.get_num_vertices()))
{
    /* Remap indices to 0,1,... indexing schema. */
    auto remap = mimir::IndexMap<mimir::Index>();
    for (const auto& vertex : digraph.get_vertices())
    {
        remap.emplace(vertex.get_index(), remap.size());
    }

    /* Add edges. */
    for (const auto& edge : digraph.get_edges())
    {
        add_edge(remap.at(edge.get_source()), remap.at(edge.get_target()));
    }

    /* Add vertex coloring. */
    auto coloring = mimir::ColorList(digraph.get_num_vertices());
    for (const auto& vertex : digraph.get_vertices())
    {
        coloring.at(remap.at(vertex.get_index())) = get_color(vertex);
    }
    add_vertex_coloring(coloring);
}

DenseGraph::DenseGraph(const DenseGraph& other) : m_impl(std::make_unique<DenseGraphImpl>(*other.m_impl)) {}

DenseGraph& DenseGraph::operator=(const DenseGraph& other)
{
    if (this != &other)
    {
        m_impl = std::make_unique<DenseGraphImpl>(*other.m_impl);
    }
    return *this;
}

DenseGraph::DenseGraph(DenseGraph&& other) noexcept : m_impl(std::move(other.m_impl)) {}

DenseGraph& DenseGraph::operator=(DenseGraph&& other) noexcept
{
    if (this != &other)
    {
        std::swap(m_impl, other.m_impl);
    }
    return *this;
}

DenseGraph::~DenseGraph() = default;

void DenseGraph::add_vertex_coloring(const mimir::ColorList& vertex_coloring) { m_impl->add_vertex_coloring(vertex_coloring); }

void DenseGraph::add_edge(size_t source, size_t target) { m_impl->add_edge(source, target); }

Certificate DenseGraph::compute_certificate() const { return m_impl->compute_certificate(); }

void DenseGraph::clear(size_t num_vertices) { m_impl->clear(num_vertices); }

bool DenseGraph::is_directed() const { return m_impl->is_directed(); }

/* SparseGraph*/

SparseGraph::SparseGraph() : m_impl(std::make_unique<SparseGraphImpl>(0)) {}

SparseGraph::SparseGraph(size_t num_vertices) : m_impl(std::make_unique<SparseGraphImpl>(num_vertices)) {}

SparseGraph::SparseGraph(const mimir::StaticVertexColoredDigraph& digraph) : m_impl(std::make_unique<SparseGraphImpl>(digraph.get_num_vertices()))
{
    /* Remap indices to 0,1,... indexing schema. */
    auto remap = mimir::IndexMap<mimir::Index>();
    for (const auto& vertex : digraph.get_vertices())
    {
        remap.emplace(vertex.get_index(), remap.size());
    }

    /* Add edges. */
    for (const auto& edge : digraph.get_edges())
    {
        add_edge(remap.at(edge.get_source()), remap.at(edge.get_target()));
    }

    /* Add vertex coloring. */
    auto coloring = mimir::ColorList(digraph.get_num_vertices());
    for (const auto& vertex : digraph.get_vertices())
    {
        coloring.at(remap.at(vertex.get_index())) = get_color(vertex);
    }
    add_vertex_coloring(coloring);
}

SparseGraph::SparseGraph(const SparseGraph& other) : m_impl(std::make_unique<SparseGraphImpl>(*other.m_impl)) {}

SparseGraph& SparseGraph::operator=(const SparseGraph& other)
{
    if (this != &other)
    {
        m_impl = std::make_unique<SparseGraphImpl>(*other.m_impl);
    }
    return *this;
}

SparseGraph::SparseGraph(SparseGraph&& other) noexcept : m_impl(std::move(other.m_impl)) {}

SparseGraph& SparseGraph::operator=(SparseGraph&& other) noexcept
{
    if (this != &other)
    {
        std::swap(m_impl, other.m_impl);
    }
    return *this;
}

SparseGraph::~SparseGraph() = default;

void SparseGraph::add_vertex_coloring(const mimir::ColorList& vertex_coloring) { m_impl->add_vertex_coloring(vertex_coloring); }

void SparseGraph::add_edge(size_t source, size_t target) { m_impl->add_edge(source, target); }

Certificate SparseGraph::compute_certificate() { return m_impl->compute_certificate(); }

void SparseGraph::clear(size_t num_vertices) { m_impl->clear(num_vertices); }

bool SparseGraph::is_directed() const { return m_impl->is_directed(); }
}

size_t std::hash<nauty_wrapper::Certificate>::operator()(const nauty_wrapper::Certificate& element) const
{
    return mimir::hash_combine(element.get_canonical_graph(), element.get_canonical_coloring());
}
