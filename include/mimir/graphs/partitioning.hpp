/*
 * Copyright (C) 2023 Dominik Drexler and Simon Stahlberg
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

#ifndef MIMIR_GRAPHS_PARTITIONING_HPP_
#define MIMIR_GRAPHS_PARTITIONING_HPP_

#include <span>
#include <vector>

namespace mimir
{

class Partitioning
{
private:
    std::vector<int> m_vertex_index_permutation;
    // Vertex partitioning, uses nauty's representation, see their documentation.
    // e.g. permutation 1 4 2 3 0
    // and partitioning 1 0 1 1 0
    // represents the vertex partitioning {{1,4},{2,3,0}}
    std::vector<int> m_partitioning;
    // Store the begin positions of the vertex indices in the permutation
    // In the above example, we get 0 2 5
    std::vector<int> m_partition_begin;

    friend class ObjectGraphFactory;

public:
    Partitioning();
    Partitioning(std::vector<int> vertex_index_permutation, std::vector<int> partitioning);

    const std::vector<int>& get_vertex_index_permutation() const;
    const std::vector<int>& get_partitioning() const;

    std::span<const int> get_partition(size_t partition_index) const;
};

}

#endif
