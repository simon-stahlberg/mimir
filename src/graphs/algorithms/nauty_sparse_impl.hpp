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

#ifndef SRC_GRAPHS_ALGORITHMS_NAUTY_SPARSE_IMPL_HPP_
#define SRC_GRAPHS_ALGORITHMS_NAUTY_SPARSE_IMPL_HPP_

// Only include nauty_sparse_impl.hpp in a source file to avoid transitive includes of nauty.h.
#include "mimir/graphs/algorithms/nauty.hpp"
#include "mimir/graphs/declarations.hpp"

#include <nausparse.h>
#include <nauty.h>
#include <span>
#include <sstream>
#include <string>
#include <vector>

namespace mimir::graphs::nauty::details
{

class SparseGraphImpl
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
    std::vector<uint32_t> m_coloring;

    // The nauty graph that consumes the data above.
    sparsegraph m_graph;

    void initialize_sparsegraph();

public:
    SparseGraphImpl(size_t nde,
                    std::vector<size_t> v,
                    int nv,
                    std::vector<int> d,
                    std::vector<int> e,
                    size_t vlen,
                    size_t dlen,
                    size_t elen,
                    std::vector<int> lab,
                    std::vector<int> ptn,
                    std::vector<uint32_t> coloring);

    SparseGraphImpl(const SparseGraphImpl& other);
    SparseGraphImpl& operator=(const SparseGraphImpl& other);
    SparseGraphImpl(SparseGraphImpl&& other) noexcept = default;
    SparseGraphImpl& operator=(SparseGraphImpl&& other) noexcept = default;

    SparseGraphImpl compute_canonical_graph();

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
    const std::vector<uint32_t>& get_coloring() const;
};

extern std::ostream& operator<<(std::ostream& out, const SparseGraphImpl& graph);

}

#endif
