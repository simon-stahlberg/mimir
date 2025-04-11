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
    SparseGraph(0, std::vector<size_t> {}, 0, std::vector<int> {}, std::vector<int> {}, 0, 0, 0, std::vector<int> {}, std::vector<int> {}, ColorList {})
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
                         ColorList coloring)
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
                             ColorList coloring)
{
    m_impl = std::make_shared<
        details::SparseGraphImpl>(nde, v, nv, std::move(d), std::move(e), vlen, dlen, elen, std::move(lab), std::move(ptn), std::move(coloring));
}

SparseGraph::SparseGraph(const SparseGraph& other) = default;

SparseGraph& SparseGraph::operator=(const SparseGraph& other) = default;

SparseGraph::SparseGraph(SparseGraph&& other) noexcept = default;

SparseGraph& SparseGraph::operator=(SparseGraph&& other) noexcept = default;

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

const ColorList& SparseGraph::get_coloring() const { return m_impl->get_coloring(); }

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

std::vector<int>& apply_permutation(const std::vector<int>& pi, std::vector<int>& ref_vec)
{
    for (size_t i = 0; i < ref_vec.size(); ++i)
    {
        ref_vec[i] = pi[ref_vec[i]];
    }

    return ref_vec;
}

std::vector<int> compute_lab_permutation(const SparseGraph& source, const SparseGraph& target)
{
    assert(loki::EqualTo<SparseGraph>()(source, target));

    const auto& source_lab = source.get_lab();
    const auto& target_lab = target.get_lab();

    auto target_v_idx_to_index = std::unordered_map<int, int> {};
    for (int i = 0; i < target.get_nv(); ++i)
    {
        target_v_idx_to_index[target_lab[i]] = i;
    }

    auto phi = std::vector<int>(source.get_nv());
    for (int i = 0; i < source.get_nv(); ++i)
    {
        phi[i] = target_v_idx_to_index.at(source_lab[i]);
    }

    return phi;
}

std::vector<int> compute_permutation(const SparseGraph& source, const SparseGraph& target)
{
    assert(loki::EqualTo<SparseGraph>()(source, target));

    auto permutation = std::vector<int>(source.get_nv());
    std::iota(permutation.begin(), permutation.end(), 0);

    apply_permutation(source.get_pi_inverse(), apply_permutation(compute_lab_permutation(source, target), apply_permutation(source.get_pi(), permutation)));

    return permutation;
}

}
