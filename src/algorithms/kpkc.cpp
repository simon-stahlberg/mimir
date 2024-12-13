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

#include "mimir/algorithms/kpkc.hpp"

#include <functional>
#include <iostream>
#include <limits>
#include <vector>

namespace mimir
{

mimir::generator<const std::vector<size_t>&>
find_all_k_cliques_in_k_partite_graph_helper(const std::vector<boost::dynamic_bitset<>>& adjacency_matrix,
                                             const std::vector<std::vector<size_t>>& partitions,
                                             std::vector<std::vector<boost::dynamic_bitset<>>>& compatible_verticess,
                                             boost::dynamic_bitset<>& partition_bits,
                                             std::vector<size_t>& partial_solution,
                                             size_t depth)
{
    size_t k = partitions.size();
    size_t best_set_bits = std::numeric_limits<size_t>::max();
    size_t best_partition = std::numeric_limits<size_t>::max();
    auto& compatible_vertices = compatible_verticess[depth];  // fetch current compatible vertices

    // Find the best partition to work with
    for (size_t partition = 0; partition < k; ++partition)
    {
        auto num_set_bits = compatible_vertices[partition].count();
        if (!partition_bits[partition] && (num_set_bits < best_set_bits))
        {
            best_set_bits = num_set_bits;
            best_partition = partition;
        }
    }

    // Iterate through compatible vertices in the best partition
    size_t adjacent_index = compatible_vertices[best_partition].find_first();
    while (adjacent_index < compatible_vertices[best_partition].size())
    {
        size_t vertex = partitions[best_partition][adjacent_index];
        compatible_vertices[best_partition][adjacent_index] = 0;
        partial_solution.push_back(vertex);

        if (partial_solution.size() == k)
        {
            co_yield partial_solution;
        }
        else
        {
            // Update compatible vertices for the next recursion
            auto& compatible_vertices_next = compatible_verticess[depth + 1];  // fetch next compatible vertices
            compatible_vertices_next = compatible_vertices;                    // important to set next to cur, before the update.
            size_t offset = 0;
            for (size_t partition = 0; partition < k; ++partition)
            {
                auto partition_size = compatible_vertices_next[partition].size();
                if (!partition_bits[partition])
                {
                    for (size_t index = 0; index < partition_size; ++index)
                    {
                        compatible_vertices_next[partition][index] &= adjacency_matrix[vertex][index + offset];
                    }
                }
                offset += partition_size;
            }

            partition_bits[best_partition] = 1;

            size_t possible_additions = 0;
            for (size_t partition = 0; partition < k; ++partition)
            {
                if (!partition_bits[partition] && compatible_vertices[partition].any())
                {
                    ++possible_additions;
                }
            }

            if ((partial_solution.size() + possible_additions) == k)
            {
                for (const auto& result : find_all_k_cliques_in_k_partite_graph_helper(adjacency_matrix,
                                                                                       partitions,
                                                                                       compatible_verticess,
                                                                                       partition_bits,
                                                                                       partial_solution,
                                                                                       depth + 1))
                {
                    co_yield result;
                }
            }

            partition_bits[best_partition] = 0;
        }

        partial_solution.pop_back();
        adjacent_index = compatible_vertices[best_partition].find_next(adjacent_index);
    }
}

mimir::generator<const std::vector<size_t>&> create_k_clique_in_k_partite_graph_generator(const std::vector<boost::dynamic_bitset<>>& adjacency_matrix,
                                                                                          const std::vector<std::vector<size_t>>& partitions)
{
    const size_t k = partitions.size();

    auto compatible_vertices = std::vector<boost::dynamic_bitset<>> {};
    for (std::size_t index = 0; index < k; ++index)
    {
        boost::dynamic_bitset<> bitset(partitions[index].size());
        bitset.set();
        compatible_vertices.push_back(std::move(bitset));
    }

    // Optimization 1, we preallocate k vectors of compatible_vertices of size k.
    // This is essentially a k x k matrix of dynamic_bitsets.
    auto compatible_verticess = std::vector<std::vector<boost::dynamic_bitset<>>>(k, compatible_vertices);

    boost::dynamic_bitset<> partition_bits(k);
    partition_bits.reset();
    std::vector<size_t> partial_solution;

    for (const auto& result :
         find_all_k_cliques_in_k_partite_graph_helper(adjacency_matrix, partitions, compatible_verticess, partition_bits, partial_solution, 0))
    {
        co_yield result;
    }
}

}
