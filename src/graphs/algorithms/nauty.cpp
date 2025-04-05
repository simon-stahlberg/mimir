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

#include "mimir/graphs/algorithms/nauty.hpp"

#include "mimir/common/types.hpp"
#include "nauty_sparse_impl.hpp"

#include <cassert>
#include <iostream>
#include <loki/details/utils/hash.hpp>
#include <stdexcept>

namespace mimir::graphs::nauty
{

/* SparseGraph*/

SparseGraph::SparseGraph() :
    SparseGraph(0,
                std::vector<size_t> {},
                0,
                std::vector<int> {},
                std::vector<int> {},
                0,
                0,
                0,
                std::vector<int> {},
                std::vector<int> {},
                std::vector<uint32_t> {})
{
}

SparseGraph::SparseGraph(size_t nde,
                         std::vector<size_t> v,
                         int nv,
                         std::vector<int> d,
                         std::vector<int> e,
                         size_t vlen,
                         size_t dlen,
                         size_t elen,
                         std::vector<int> lab,
                         std::vector<int> ptn,
                         std::vector<uint32_t> coloring)
{
}

void SparseGraph::initialize(size_t nde,
                             std::vector<size_t> v,
                             int nv,
                             std::vector<int> d,
                             std::vector<int> e,
                             size_t vlen,
                             size_t dlen,
                             size_t elen,
                             std::vector<int> lab,
                             std::vector<int> ptn,
                             std::vector<uint32_t> coloring)
{
    m_impl = std::make_unique<
        details::SparseGraphImpl>(nde, v, nv, std::move(d), std::move(e), vlen, dlen, elen, std::move(lab), std::move(ptn), std::move(coloring));
}

SparseGraph::SparseGraph(const SparseGraph& other) : m_impl(std::make_unique<details::SparseGraphImpl>(*other.m_impl)) {}

SparseGraph& SparseGraph::operator=(const SparseGraph& other)
{
    if (this != &other)
    {
        m_impl = std::make_unique<details::SparseGraphImpl>(*other.m_impl);
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

size_t SparseGraph::get_nde() const { return m_impl->get_nde(); }

const std::vector<size_t>& SparseGraph::get_v() const { return m_impl->get_v(); }

int SparseGraph::get_nv() const { return m_impl->get_nv(); }

const std::vector<int>& SparseGraph::get_d() const { return m_impl->get_d(); }

const std::vector<int>& SparseGraph::get_e() const { return m_impl->get_e(); }

size_t SparseGraph::get_vlen() const { return m_impl->get_vlen(); }

size_t SparseGraph::get_dlen() const { return m_impl->get_dlen(); }

size_t SparseGraph::get_elen() const { return m_impl->get_elen(); }

const std::vector<int>& SparseGraph::get_lab() const { return m_impl->get_lab(); }

const std::vector<int>& SparseGraph::get_ptn() const { return m_impl->get_ptn(); }

const std::vector<uint32_t>& SparseGraph::get_coloring() const { return m_impl->get_coloring(); }

std::ostream& operator<<(std::ostream& out, const SparseGraph& graph)
{
    out << *graph.m_impl;

    return out;
}

SparseGraph& SparseGraph::canonize()
{
    m_impl->canonize();
    return *this;
}

const std::vector<int>& SparseGraph::get_pi() const { return m_impl->get_pi(); }

const std::vector<int>& SparseGraph::get_pi_inverse() const { return m_impl->get_pi_inverse(); }

}
