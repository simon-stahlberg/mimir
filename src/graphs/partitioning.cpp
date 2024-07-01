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

#include "mimir/graphs/partitioning.hpp"

namespace mimir
{
Partitioning::Partitioning() : m_vertex_index_permutation(), m_partitioning(), m_partition_begin() { m_partition_begin.push_back(0); }

Partitioning::Partitioning(std::vector<int> vertex_index_permutation, std::vector<int> partitioning) :
    m_vertex_index_permutation(std::move(vertex_index_permutation)),
    m_partitioning(std::move(partitioning)),
    m_partition_begin()
{
    m_partition_begin.push_back(0);
    for (size_t pos = 0; pos < m_partitioning.size(); ++pos)
    {
        if (m_partitioning.at(pos) == 0)
        {
            // 0 marks the last element in the partition
            m_partition_begin.push_back(pos + 1);
        }
    }
}

const std::vector<int>& Partitioning::get_vertex_index_permutation() const { return m_vertex_index_permutation; }

const std::vector<int>& Partitioning::get_partitioning() const { return m_partitioning; }

std::span<const int> Partitioning::get_partition(size_t partition_index) const
{
    return { m_vertex_index_permutation.begin() + m_partition_begin.at(partition_index),
             m_vertex_index_permutation.begin() + m_partition_begin.at(partition_index + 1) };
}
}
