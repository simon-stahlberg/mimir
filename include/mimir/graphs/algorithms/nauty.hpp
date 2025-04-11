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

#include "mimir/common/deref_shared_ptr.hpp"
#include "mimir/common/printers.hpp"
#include "mimir/graphs/color.hpp"
#include "mimir/graphs/graph_interface.hpp"
#include "mimir/graphs/graph_properties.hpp"
#include "mimir/graphs/types.hpp"

#include <memory>
#include <ostream>
#include <vector>

/// @brief Wrap a namespace around nauty's interface
namespace mimir::graphs::nauty
{
namespace details
{
class SparseGraphImpl;
}

/// @brief `SparseGraph` encapsulates a sparse graph representation compatible with Nauty.
class SparseGraph
{
private:
    std::shared_ptr<details::SparseGraphImpl> m_impl;

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
                ColorList coloring);

    void initialize(size_t nde,
                    std::vector<size_t> v,
                    int nv,
                    std::vector<int> d,
                    std::vector<int> e,
                    size_t vlen,
                    size_t dlen,
                    size_t elen,
                    std::vector<int> lab,
                    std::vector<int> ptn,
                    ColorList coloring);

public:
    SparseGraph();

    template<typename Graph>
        requires IsVertexListGraph<Graph> && IsEdgeListGraph<Graph> && IsVertexColoredGraph<Graph>
    explicit SparseGraph(const Graph& graph) : SparseGraph()
    {
        if (is_multi_graph(graph))
        {
            throw std::runtime_error("SparseGraph::SparseGraph(graph): Multi graph is not supported.");
        }
        if (!is_loopless(graph))
        {
            throw std::runtime_error("SparseGraph::SparseGraph(graph): looping edges are not supported.");
        }
        if (!is_undirected_graph(graph))
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
            color_vertex_pairs.emplace_back(get_color(vertex), remap.at(vertex.get_index()));
        }
        std::sort(color_vertex_pairs.begin(), color_vertex_pairs.end());

        auto lab = std::vector<int>(nv, 0);
        auto ptn = std::vector<int>(nv, 0);
        auto coloring = ColorList {};
        coloring.reserve(nv);
        for (int i = 1; i < nv; ++i)
        {
            lab[i - 1] = color_vertex_pairs[i - 1].second;
            ptn[i - 1] = (color_vertex_pairs[i - 1].first != color_vertex_pairs[i].first) ? 0 : 1;
            coloring.push_back(color_vertex_pairs[i - 1].first);
        }
        lab[nv - 1] = color_vertex_pairs[nv - 1].second;
        ptn[nv - 1] = 0;

        initialize(nde, v, nv, d, e, vlen, dlen, elen, lab, ptn, coloring);
    }

    SparseGraph(const SparseGraph& other);
    SparseGraph& operator=(const SparseGraph& other);
    SparseGraph(SparseGraph&& other) noexcept;
    SparseGraph& operator=(SparseGraph&& other) noexcept;
    ~SparseGraph();

    SparseGraph& canonize();

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
    const ColorList& get_coloring() const;

    /// @brief Return vertex permutation from input graph to canonical graphs.
    /// Throws an exception if canonize() was not called before.
    /// @return
    const std::vector<int>& get_pi() const;
    /// @brief Return permutation from canonical graph to input graph.
    /// Throws an exception if canonize() was not called before.
    /// @return
    const std::vector<int>& get_pi_inverse() const;

    auto identifying_members() const
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
extern std::vector<int> compute_lab_permutation(const SparseGraph& source, const SparseGraph& target);

extern std::vector<int> compute_permutation(const SparseGraph& source, const SparseGraph& target);

extern std::ostream& operator<<(std::ostream& out, const SparseGraph& graph);

}

#endif
