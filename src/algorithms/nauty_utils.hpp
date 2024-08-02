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

#ifndef SRC_ALGORITHMS_NAUTY_UTILS_HPP_
#define SRC_ALGORITHMS_NAUTY_UTILS_HPP_

#include "mimir/common/printers.hpp"
#include "mimir/graphs/declarations.hpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <vector>

namespace nauty_wrapper
{

/// @brief Compute lab and ptn from vertex coloring.
/// @param vertex_coloring
/// @param ref_lab
/// @param ref_ptn
/// @param n
inline void initialize_lab_and_ptr(const mimir::ColorList& vertex_coloring, std::vector<int>& ref_lab, std::vector<int>& ref_ptn)
{
    const auto n = vertex_coloring.size();
    assert(ref_lab.size() >= n);
    assert(ref_ptn.size() >= n);

    if (n > 0)
    {
        using ColorVertexPair = std::pair<uint32_t, uint32_t>;
        std::vector<ColorVertexPair> color_vertex_pairs;
        color_vertex_pairs.reserve(n);
        for (size_t i = 0; i < n; ++i)
        {
            color_vertex_pairs.emplace_back(vertex_coloring[i], i);
        }
        std::sort(color_vertex_pairs.begin(), color_vertex_pairs.end());
        for (size_t i = 1; i < n; ++i)
        {
            ref_lab[i - 1] = color_vertex_pairs[i - 1].second;
            ref_ptn[i - 1] = (color_vertex_pairs[i - 1].first != color_vertex_pairs[i].first) ? 0 : 1;
        }
        ref_lab[n - 1] = color_vertex_pairs[n - 1].second;
        ref_ptn[n - 1] = 0;
    }

    // std::cout << "vertex_coloring: ";
    // mimir::operator<<(std::cout, vertex_coloring);
    // std::cout << std::endl;
    // std::cout << "lab: ";
    // mimir::operator<<(std::cout, ref_lab);
    // std::cout << std::endl;
    // std::cout << "ptn: ";
    // mimir::operator<<(std::cout, ref_ptn);
    // std::cout << std::endl;
}
}

#endif
