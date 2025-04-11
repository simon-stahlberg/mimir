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

#include "mimir/common/deref_shared_ptr.hpp"
#include "mimir/graphs/color.hpp"

#include <nausparse.h>
#include <nauty.h>
#include <ostream>
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
    ColorList m_coloring;

    // The nauty graph that consumes the data above.
    sparsegraph m_graph;

    bool m_is_canonical;
    std::vector<int> m_pi;
    std::vector<int> m_pi_inverse;

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
                    ColorList coloring);

    SparseGraphImpl(const SparseGraphImpl& other);
    SparseGraphImpl& operator=(const SparseGraphImpl& other);
    SparseGraphImpl(SparseGraphImpl&& other) noexcept = default;
    SparseGraphImpl& operator=(SparseGraphImpl&& other) noexcept = default;

    void canonize();

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
};

extern std::ostream& operator<<(std::ostream& out, const SparseGraphImpl& graph);

}

#endif
