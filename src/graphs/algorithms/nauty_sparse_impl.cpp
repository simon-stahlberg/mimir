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

#include "mimir/common/printers.hpp"

#include <iostream>

namespace mimir::graphs::nauty::details
{

void SparseGraphImpl::initialize_sparsegraph()
{
    SG_INIT(m_graph);
    m_graph.nde = m_nde;
    m_graph.v = m_v.data();
    m_graph.nv = m_nv;
    m_graph.d = m_d.data();
    m_graph.e = m_e.data();
    m_graph.vlen = m_vlen;
    m_graph.dlen = m_dlen;
    m_graph.elen = m_elen;
}

SparseGraphImpl::SparseGraphImpl(size_t nde,
                                 std::vector<size_t> v,
                                 int nv,
                                 std::vector<int> d,
                                 std::vector<int> e,
                                 size_t vlen,
                                 size_t dlen,
                                 size_t elen,
                                 std::vector<int> lab,
                                 std::vector<int> ptn,
                                 ColorList coloring) :
    m_nde(nde),
    m_v(std::move(v)),
    m_nv(nv),
    m_d(std::move(d)),
    m_e(std::move(e)),
    m_vlen(vlen),
    m_dlen(dlen),
    m_elen(elen),
    m_lab(std::move(lab)),
    m_ptn(std::move(ptn)),
    m_coloring(std::move(coloring)),
    m_graph(),
    m_is_canonical(false)
{
    initialize_sparsegraph();
}

SparseGraphImpl::SparseGraphImpl(const SparseGraphImpl& other) :
    m_nde(other.get_nde()),
    m_v(other.get_v()),
    m_nv(other.get_nv()),
    m_d(other.get_d()),
    m_e(other.get_e()),
    m_vlen(other.get_vlen()),
    m_dlen(other.get_dlen()),
    m_elen(other.get_elen()),
    m_lab(other.get_lab()),
    m_ptn(other.get_ptn()),
    m_coloring(other.get_coloring()),
    m_graph(),
    m_is_canonical(other.m_is_canonical)
{
    initialize_sparsegraph();
}

SparseGraphImpl& SparseGraphImpl::operator=(const SparseGraphImpl& other)
{
    if (this != &other)
    {
        m_nde = other.get_nde();
        m_v = other.get_v();
        m_nv = other.get_nv();
        m_d = other.get_d();
        m_e = other.get_e();
        m_vlen = other.get_vlen();
        m_dlen = other.get_dlen();
        m_elen = other.get_elen();
        m_lab = other.get_lab();
        m_ptn = other.get_ptn();
        m_graph = sparsegraph();
        m_is_canonical = other.m_is_canonical;
        initialize_sparsegraph();
    }

    return *this;
}

size_t SparseGraphImpl::get_nde() const { return m_nde; }

const std::vector<size_t>& SparseGraphImpl::get_v() const { return m_v; }

int SparseGraphImpl::get_nv() const { return m_nv; }

const std::vector<int>& SparseGraphImpl::get_d() const { return m_d; }

const std::vector<int>& SparseGraphImpl::get_e() const { return m_e; }

size_t SparseGraphImpl::get_vlen() const { return m_vlen; }

size_t SparseGraphImpl::get_dlen() const { return m_dlen; }

size_t SparseGraphImpl::get_elen() const { return m_elen; }

const std::vector<int>& SparseGraphImpl::get_lab() const { return m_lab; }

const std::vector<int>& SparseGraphImpl::get_ptn() const { return m_ptn; }

const ColorList& SparseGraphImpl::get_coloring() const { return m_coloring; }

const std::vector<int>& SparseGraphImpl::get_pi() const
{
    if (!m_is_canonical)
    {
        throw std::runtime_error("SparseGraphImpl::get_pi(): Expected canonical form.");
    }
    return m_pi;
}

const std::vector<int>& SparseGraphImpl::get_pi_inverse() const
{
    if (!m_is_canonical)
    {
        throw std::runtime_error("SparseGraphImpl::get_pi(): Expected canonical form.");
    }
    return m_pi_inverse;
}

void SparseGraphImpl::canonize()
{
    if (m_is_canonical)
        return;
    m_is_canonical = true;

    static DEFAULTOPTIONS_SPARSEGRAPH(options);
    options.defaultptn = FALSE;
    options.getcanon = TRUE;
    options.digraph = FALSE;
    options.writeautoms = FALSE;

    auto orbits = std::vector<int>(m_nv);

    statsblk stats;

    auto canon_graph = SparseGraphImpl(*this);

    // std::cout << "Canongraph before: " << canon_graph << std::endl;

    sparsenauty(&m_graph, canon_graph.m_lab.data(), canon_graph.m_ptn.data(), orbits.data(), &options, &stats, &canon_graph.m_graph);

    // According to documentation:
    //   canon_graph has contiguous adjacency lists that are not necessarily sorted
    sortlists_sg(&canon_graph.m_graph);

    // std::cout << "Canongraph after: " << canon_graph << std::endl;

    auto label_to_index = std::unordered_map<int, int> {};
    auto canon_label_to_index = std::unordered_map<int, int> {};
    for (int i = 0; i < get_nv(); ++i)
    {
        label_to_index.emplace(m_lab[i], i);
        canon_label_to_index.emplace(canon_graph.m_lab[i], i);
    }

    canon_graph.m_pi.resize(m_nv);
    canon_graph.m_pi_inverse.resize(m_nv);
    for (int i = 0; i < m_nv; ++i)
    {
        canon_graph.m_pi[i] = label_to_index[canon_graph.m_lab[i]];    // pi maps original -> canonical
        canon_graph.m_pi_inverse[i] = canon_label_to_index[m_lab[i]];  // pi_inverse maps canonical -> original
    }

    // std::cout << "pi: ";
    // mimir::operator<<(std::cout, canon_graph.m_pi);
    // std::cout << std::endl;
    // std::cout << "pi_inverse: ";
    // mimir::operator<<(std::cout, canon_graph.m_pi_inverse);
    // std::cout << std::endl;

    std::swap(*this, canon_graph);
}

std::ostream& operator<<(std::ostream& out, const SparseGraphImpl& graph)
{
    out << "nde:" << graph.get_nde() << "\n"
        << "v: ";
    mimir::operator<<(out, graph.get_v());
    out << "\n"
        << "nv:" << graph.get_nv() << "\n"
        << "d: ";
    mimir::operator<<(out, graph.get_d());
    out << "\n"
        << "e: ";
    mimir::operator<<(out, graph.get_e());
    out << "\n"
        << "vlen: " << graph.get_vlen() << "\n"
        << "dlen: " << graph.get_dlen() << "\n"
        << "elen: " << graph.get_elen() << "\n"
        << "lab: ";
    mimir::operator<<(out, graph.get_lab());
    out << "\n"
        << "ptn: ";
    mimir::operator<<(out, graph.get_ptn());
    out << "\n"
        << "coloring: ";
    mimir::operator<<(out, graph.get_coloring());

    return out;
}
}
