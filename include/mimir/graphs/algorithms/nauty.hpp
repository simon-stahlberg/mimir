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
#include "mimir/graphs/graph_interface.hpp"
#include "mimir/graphs/graph_properties.hpp"
#include "mimir/graphs/property.hpp"
#include "mimir/graphs/types.hpp"

#include <memory>
#include <nausparse.h>
#include <nauty.h>
#include <ostream>
#include <vector>

/// @brief Wrap a namespace around nauty's interface
namespace mimir::graphs::nauty
{

/// @brief `SparseGraph` encapsulates a sparse graph representation compatible with Nauty.
template<Property... Ts>
class SparseGraph
{
private:
    size_t m_nde;
    std::vector<size_t> m_v;
    int m_nv;
    std::vector<int> m_d;
    std::vector<int> m_e;
    size_t m_vlen;
    size_t m_dlen;
    size_t m_elen;
    std::vector<int> m_lab;
    std::vector<int> m_ptn;
    PropertiesList<Ts...> m_coloring;

    // The nauty graph that consumes the data above.
    sparsegraph m_graph;

    bool m_is_canonical;
    std::vector<int> m_pi;
    std::vector<int> m_pi_inverse;

    void initialize_sparsegraph();

public:
    SparseGraph();

    SparseGraph(size_t nde,
                std::vector<size_t> v,
                int nv,
                std::vector<int> d,
                std::vector<int> e,
                size_t vlen,
                size_t dlen,
                size_t elen,
                std::vector<int> lab,
                std::vector<int> ptn,
                PropertiesList<Ts...> coloring);

    template<typename Graph>
        requires IsVertexListGraph<Graph> && IsEdgeListGraph<Graph>
    explicit SparseGraph(const Graph& graph) : SparseGraph()
    {
        if (is_multi(graph))
        {
            throw std::runtime_error("SparseGraph::SparseGraph(graph): Multi graph is not supported.");
        }
        if (!is_loopless(graph))
        {
            throw std::runtime_error("SparseGraph::SparseGraph(graph): looping edges are not supported.");
        }
        if (!is_undirected(graph))
        {
            throw std::runtime_error("SparseGraph::SparseGraph(graph): directed graphs are not supported due to bad performance. Please translate it into an "
                                     "undirected graph before using nauty.");
        }

        /* Remap indices to 0,1,... indexing schema. */
        auto remap = mimir::IndexMap<uint32_t>();
        for (const auto& vertex : graph.get_vertices())
        {
            remap.emplace(vertex.get_index(), remap.size());
        }

        // Copies from nauty sparsegraph and wrapped into c++ types.
        size_t nde;              /* Number of directed edges (loops contribute only 1) */
        std::vector<size_t> v;   /* Array of indexes into e[*] */
        int nv;                  /* Number of vertices */
        std::vector<int> d;      /* Array with out-degree of each vertex */
        std::vector<int> e;      /* Array to hold lists of neighbours */
        size_t vlen, dlen, elen; /* Sizes of arrays in units of type */

        nv = graph.get_num_vertices(); /* Number of directed edges (loops contribute only 1) */
        nde = graph.get_num_edges();   /* Number of vertices */
        vlen = nv;
        dlen = nv;
        elen = nde;

        d.reserve(nv);
        v.reserve(nv);
        e.reserve(nde);
        for (const auto& vertex : graph.get_vertices())
        {
            d.push_back(graph.template get_degree<ForwardTag>(vertex.get_index()));
            v.push_back(e.size());
            for (const auto& edge : graph.template get_adjacent_edges<ForwardTag>(vertex.get_index()))
            {
                e.push_back(remap.at(edge.get_target()));
            }
        }

        /* Add vertex coloring. */

        auto color_vertex_pairs = std::vector<std::pair<Color, uint32_t>> {};
        color_vertex_pairs.reserve(nv);
        for (const auto& vertex : graph.get_vertices())
        {
            color_vertex_pairs.emplace_back(vertex.get_properties(), remap.at(vertex.get_index()));
        }
        std::sort(color_vertex_pairs.begin(), color_vertex_pairs.end());

        auto coloring = PropertiesList<Ts...> {};
        coloring.reserve(nv);
        for (const auto& [color, _] : color_vertex_pairs)
        {
            coloring.push_back(color);
        }

        auto lab = std::vector<int>(nv, 0);
        auto ptn = std::vector<int>(nv, 0);
        for (int i = 1; i < nv; ++i)
        {
            lab[i - 1] = color_vertex_pairs[i - 1].second;
            ptn[i - 1] = (color_vertex_pairs[i - 1].first != color_vertex_pairs[i].first) ? 0 : 1;
        }
        if (nv > 0)
        {
            lab[nv - 1] = color_vertex_pairs[nv - 1].second;
            ptn[nv - 1] = 0;
        }

        initialize(nde, v, nv, d, e, vlen, dlen, elen, lab, ptn, coloring);
    }

    SparseGraph(const SparseGraph& other);
    SparseGraph& operator=(const SparseGraph& other);
    SparseGraph(SparseGraph&& other) noexcept;
    SparseGraph& operator=(SparseGraph&& other) noexcept;
    ~SparseGraph();

    void canonize();

    friend std::ostream& operator<<(std::ostream& out, const SparseGraph& graph);

    size_t get_nde() const;
    const std::vector<size_t>& get_v() const;
    int get_nv() const;
    const std::vector<int>& get_d() const;
    const std::vector<int>& get_e() const;
    size_t get_vlen() const;
    size_t get_dlen() const;
    size_t get_elen() const;
    const std::vector<int>& get_lab() const;
    const std::vector<int>& get_ptn() const;
    const PropertiesList<Ts...>& get_coloring() const;

    /// @brief Return vertex permutation from input graph to canonical graphs.
    /// Throws an exception if canonize() was not called before.
    /// @return
    const std::vector<int>& get_pi() const;
    /// @brief Return permutation from canonical graph to input graph.
    /// Throws an exception if canonize() was not called before.
    /// @return
    const std::vector<int>& get_pi_inverse() const;

    auto identifying_members() const noexcept
    {
        return std::tuple(get_nde(),
                          std::cref(get_v()),
                          get_nv(),
                          std::cref(get_d()),
                          std::cref(get_e()),
                          get_vlen(),
                          get_dlen(),
                          get_elen(),
                          std::cref(get_coloring()));
    }
};

extern std::vector<int>& apply_permutation(const std::vector<int>& pi, std::vector<int>& ref_vec);

/// @brief Compute the permutation induced by the isomorphism from the source `SparseGraph` to the target `SparseGraph`.
/// Assumes that there exists an isomorphism from source to target.
/// @param source the source `SparseGraph` in the permutation.
/// @param target the target `SparseGraph` in the permutation.
/// @return
template<Property... Ts>
extern std::vector<int> compute_lab_permutation(const SparseGraph<Ts...>& source, const SparseGraph<Ts...>& target);

template<Property... Ts>
extern std::vector<int> compute_permutation(const SparseGraph<Ts...>& source, const SparseGraph<Ts...>& target);

template<Property... Ts>
extern std::ostream& operator<<(std::ostream& out, const SparseGraph<Ts...>& graph);

/**
 * Implementations
 */

template<Property... Ts>
void SparseGraph<Ts...>::initialize_sparsegraph()
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

template<Property... Ts>
SparseGraph<Ts...>::SparseGraph(size_t nde,
                                std::vector<size_t> v,
                                int nv,
                                std::vector<int> d,
                                std::vector<int> e,
                                size_t vlen,
                                size_t dlen,
                                size_t elen,
                                std::vector<int> lab,
                                std::vector<int> ptn,
                                PropertiesList<Ts...> coloring) :
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

template<Property... Ts>
SparseGraph<Ts...>::SparseGraph(const SparseGraph<Ts...>& other) :
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

template<Property... Ts>
SparseGraph<Ts...>& SparseGraph<Ts...>::operator=(const SparseGraph<Ts...>& other)
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

template<Property... Ts>
size_t SparseGraph<Ts...>::get_nde() const
{
    return m_nde;
}

template<Property... Ts>
const std::vector<size_t>& SparseGraph<Ts...>::get_v() const
{
    return m_v;
}

template<Property... Ts>
int SparseGraph<Ts...>::get_nv() const
{
    return m_nv;
}

template<Property... Ts>
const std::vector<int>& SparseGraph<Ts...>::get_d() const
{
    return m_d;
}

template<Property... Ts>
const std::vector<int>& SparseGraph<Ts...>::get_e() const
{
    return m_e;
}

template<Property... Ts>
size_t SparseGraph<Ts...>::get_vlen() const
{
    return m_vlen;
}

template<Property... Ts>
size_t SparseGraph<Ts...>::get_dlen() const
{
    return m_dlen;
}

template<Property... Ts>
size_t SparseGraph<Ts...>::get_elen() const
{
    return m_elen;
}

template<Property... Ts>
const std::vector<int>& SparseGraph<Ts...>::get_lab() const
{
    return m_lab;
}

template<Property... Ts>
const std::vector<int>& SparseGraph<Ts...>::get_ptn() const
{
    return m_ptn;
}

template<Property... Ts>
const PropertiesList<Ts...>& SparseGraph<Ts...>::get_coloring() const
{
    return m_coloring;
}

template<Property... Ts>
const std::vector<int>& SparseGraph<Ts...>::get_pi() const
{
    if (!m_is_canonical)
    {
        throw std::runtime_error("SparseGraph<Ts...>get_pi(): Expected canonical form.");
    }
    return m_pi;
}

template<Property... Ts>
const std::vector<int>& SparseGraph<Ts...>::get_pi_inverse() const
{
    if (!m_is_canonical)
    {
        throw std::runtime_error("SparseGraph<Ts...>get_pi(): Expected canonical form.");
    }
    return m_pi_inverse;
}

template<Property... Ts>
void SparseGraph<Ts...>::canonize()
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

    auto canon_graph = SparseGraph(*this);

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

template<Property... Ts>
std::ostream& operator<<(std::ostream& out, const SparseGraph<Ts...>& graph)
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

#endif
