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

KPKCInternalMemory::KPKCInternalMemory(size_t k) :
    partition_bits(k),
    partial_solution(),
    k_compatible_vertices(std::vector<std::vector<boost::dynamic_bitset<>>>(k, std::vector<boost::dynamic_bitset<>>(k)))
{
}

void KPKCInternalMemory::verify_memory_layout(size_t k_)
{
    if (partition_bits.size() != k_)
    {
        throw std::runtime_error("KPKCInternalMemory::verify_memory_layout: expected partition_bits of size " + std::to_string(k_));
    }

    if (k_compatible_vertices.size() != k_
        && !std::all_of(k_compatible_vertices.begin(), k_compatible_vertices.end(), [k_](const auto& element) { return element.size() == k_; }))
    {
        throw std::runtime_error("KPKCInternalMemory::verify_memory_layout: expected k x k compatible_vertices.");
    }
}

mimir::generator<const std::vector<size_t>&> find_all_k_cliques_in_k_partite_graph_helper(const std::vector<boost::dynamic_bitset<>>& adjacency_matrix,
                                                                                          const std::vector<std::vector<size_t>>& partitions,
                                                                                          KPKCInternalMemory& memory,
                                                                                          size_t depth)
{
    size_t k = partitions.size();
    size_t best_set_bits = std::numeric_limits<size_t>::max();
    size_t best_partition = std::numeric_limits<size_t>::max();
    auto& compatible_vertices = memory.k_compatible_vertices[depth];  // fetch current compatible vertices

    // Find the best partition to work with
    for (size_t partition = 0; partition < k; ++partition)
    {
        auto num_set_bits = compatible_vertices[partition].count();
        if (!memory.partition_bits[partition] && (num_set_bits < best_set_bits))
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
        memory.partial_solution.push_back(vertex);

        if (memory.partial_solution.size() == k)
        {
            co_yield memory.partial_solution;
        }
        else
        {
            // Update compatible vertices for the next recursion
            auto& compatible_vertices_next = memory.k_compatible_vertices[depth + 1];  // fetch next compatible vertices
            compatible_vertices_next = compatible_vertices;                            // important to set next to cur, before the update.
            size_t offset = 0;
            for (size_t partition = 0; partition < k; ++partition)
            {
                auto partition_size = compatible_vertices_next[partition].size();
                if (!memory.partition_bits[partition])
                {
                    for (size_t index = 0; index < partition_size; ++index)
                    {
                        compatible_vertices_next[partition][index] &= adjacency_matrix[vertex][index + offset];
                    }
                }
                offset += partition_size;
            }

            memory.partition_bits[best_partition] = 1;

            size_t possible_additions = 0;
            for (size_t partition = 0; partition < k; ++partition)
            {
                if (!memory.partition_bits[partition] && compatible_vertices[partition].any())
                {
                    ++possible_additions;
                }
            }

            if ((memory.partial_solution.size() + possible_additions) == k)
            {
                for (const auto& result : find_all_k_cliques_in_k_partite_graph_helper(adjacency_matrix, partitions, memory, depth + 1))
                {
                    co_yield result;
                }
            }

            memory.partition_bits[best_partition] = 0;
        }

        memory.partial_solution.pop_back();
        adjacent_index = compatible_vertices[best_partition].find_next(adjacent_index);
    }
}

mimir::generator<const std::vector<size_t>&> create_k_clique_in_k_partite_graph_generator(const std::vector<boost::dynamic_bitset<>>& adjacency_matrix,
                                                                                          const std::vector<std::vector<size_t>>& partitions,
                                                                                          KPKCInternalMemory* memory)
{
    const size_t k = partitions.size();

    /* Get and verify or create memory layout. */
    auto managed_memory = std::unique_ptr<KPKCInternalMemory> {};
    if (!memory)
    {
        // If no external data is provided, manage data internally
        managed_memory = std::make_unique<KPKCInternalMemory>(k);
        memory = managed_memory.get();
    }
    memory->verify_memory_layout(k);

    /* Initialize the memory. */
    for (std::size_t index = 0; index < k; ++index)
    {
        memory->k_compatible_vertices[0][index].set(k, true);
    }
    memory->partition_bits.reset();
    memory->partial_solution.clear();

    /* Compute k-cliques. */
    for (const auto& result : find_all_k_cliques_in_k_partite_graph_helper(adjacency_matrix, partitions, *memory, 0))
    {
        co_yield result;
    }
}

}
