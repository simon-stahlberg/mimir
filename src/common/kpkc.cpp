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

#include <limits>

namespace mimir
{
// TODO: Try to replace data-structures with flatmemory implementations.
void find_all_k_cliques_in_k_partite_graph_helper(const std::vector<boost::dynamic_bitset<>>& adjacency_matrix,
                                                  const std::vector<std::vector<size_t>>& partitions,
                                                  std::vector<boost::dynamic_bitset<>>& compatible_vertices,
                                                  boost::dynamic_bitset<>& partition_bits,
                                                  boost::dynamic_bitset<>& not_partition_bits,  // TODO: Check if !partition_bits[i] is better...
                                                  std::vector<size_t>& partial_solution,
                                                  std::vector<std::vector<std::size_t>>& out_cliques)
{
    size_t k = partitions.size();
    size_t best_set_bits = std::numeric_limits<size_t>::max();
    size_t best_partition = std::numeric_limits<size_t>::max();

    // Find the best partition to work with
    for (size_t partition = 0; partition < k; ++partition)
    {
        const auto num_set_bits = compatible_vertices[partition].count();

        if (not_partition_bits[partition] && (num_set_bits < best_set_bits))
        {
            best_set_bits = num_set_bits;
            best_partition = partition;
        }
    }

    size_t adjacent_index = compatible_vertices[best_partition].find_first();

    // Iterate through compatible vertices in the best partition
    while (adjacent_index < compatible_vertices[best_partition].size())
    {
        size_t vertex = partitions[best_partition][adjacent_index];
        compatible_vertices[best_partition][adjacent_index] = 0;
        partial_solution.push_back(vertex);

        if (partial_solution.size() == k)
        {
            out_cliques.push_back(partial_solution);
        }
        else
        {
            // Update compatible vertices for the next recursion
            std::vector<boost::dynamic_bitset<>> compatible_vertices_next = compatible_vertices;
            size_t offset = 0;
            for (size_t partition = 0; partition < k; ++partition)
            {
                const auto partition_size = compatible_vertices_next[partition].size();
                if (not_partition_bits[partition])
                {
                    for (size_t index = 0; index < partition_size; ++index)
                    {
                        compatible_vertices_next[partition][index] &= adjacency_matrix[vertex][index + offset];
                    }
                }
                offset += partition_size;
            }

            partition_bits[best_partition] = 1;
            not_partition_bits[best_partition] = 0;

            size_t possible_additions = 0;
            for (size_t partition = 0; partition < k; ++partition)
            {
                if (not_partition_bits[partition] && compatible_vertices[partition].any())
                {
                    ++possible_additions;
                }
            }

            if ((partial_solution.size() + possible_additions) == k)
            {
                find_all_k_cliques_in_k_partite_graph_helper(adjacency_matrix,
                                                             partitions,
                                                             compatible_vertices_next,
                                                             partition_bits,
                                                             not_partition_bits,
                                                             partial_solution,
                                                             out_cliques);
            }

            partition_bits[best_partition] = 0;
            not_partition_bits[best_partition] = 1;
        }

        partial_solution.pop_back();
        adjacent_index = compatible_vertices[best_partition].find_next(adjacent_index);
    }
}

void find_all_k_cliques_in_k_partite_graph(const std::vector<boost::dynamic_bitset<>>& adjacency_matrix,
                                           const std::vector<std::vector<size_t>>& partitions,
                                           std::vector<std::vector<std::size_t>>& out_cliques)
{
    std::vector<boost::dynamic_bitset<>> compatible_vertices;

    for (std::size_t index = 0; index < partitions.size(); ++index)
    {
        boost::dynamic_bitset<> bitset(partitions[index].size());
        bitset.set();
        compatible_vertices.push_back(std::move(bitset));
    }

    const size_t k = partitions.size();
    boost::dynamic_bitset<> partition_bits(k);
    boost::dynamic_bitset<> not_partition_bits(k);
    partition_bits.reset();
    not_partition_bits.set();
    std::vector<size_t> partial_solution;

    find_all_k_cliques_in_k_partite_graph_helper(adjacency_matrix,
                                                 partitions,
                                                 compatible_vertices,
                                                 partition_bits,
                                                 not_partition_bits,
                                                 partial_solution,
                                                 out_cliques);
}

}
