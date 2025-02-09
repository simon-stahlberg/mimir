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

KPKCWorkspace::KPKCWorkspace(const std::vector<std::vector<uint32_t>>& partitions) :
    partition_bits(partitions.size()),
    partial_solution(),
    k_compatible_vertices(partitions.size(), std::vector<boost::dynamic_bitset<>>(partitions.size()))
{
    const auto k = partitions.size();

    for (uint32_t k1 = 0; k1 < k; ++k1)
    {
        for (uint32_t k2 = 0; k2 < k; ++k2)
        {
            k_compatible_vertices[k1][k2].resize(partitions[k2].size());
        }
    }

    initialize_memory(partitions);
}

void KPKCWorkspace::initialize_memory(const std::vector<std::vector<uint32_t>>& partitions)
{
    verify_memory_layout(partitions);

    const auto k = partitions.size();

    for (std::uint32_t index = 0; index < k; ++index)
    {
        k_compatible_vertices.front()[index].set();
    }
    partition_bits.reset();
    partial_solution.clear();
}

void KPKCWorkspace::verify_memory_layout(const std::vector<std::vector<uint32_t>>& partitions)
{
    const auto k = partitions.size();

    if (partition_bits.size() != k)
    {
        throw std::runtime_error("KPKCWorkspace::verify_memory_layout: expected partition_bits of size " + std::to_string(k));
    }

    if (k_compatible_vertices.size() != k)
    {
        throw std::runtime_error("KPKCWorkspace::verify_memory_layout: expected compatible_vertices to have first dimension of size " + std::to_string(k));
    }

    if (!std::all_of(k_compatible_vertices.begin(), k_compatible_vertices.end(), [k](const auto& element) { return element.size() == k; }))
    {
        throw std::runtime_error("KPKCWorkspace::verify_memory_layout: expected compatible_vertices to have second dimension of size " + std::to_string(k));
    }

    for (uint32_t k1 = 0; k1 < k; ++k1)
    {
        for (uint32_t k2 = 0; k2 < k; ++k2)
        {
            if (k_compatible_vertices[k1][k2].size() != partitions[k2].size())
            {
                throw std::runtime_error("KPKCWorkspace::verify_memory_layout: expected bitsets to match partition sizes.");
            }
        }
    }
}

mimir::generator<const std::vector<uint32_t>&> find_all_k_cliques_in_k_partite_graph_helper(const std::vector<boost::dynamic_bitset<>>& adjacency_matrix,
                                                                                            const std::vector<std::vector<uint32_t>>& partitions,
                                                                                            KPKCWorkspace& memory,
                                                                                            uint32_t depth)
{
    uint32_t k = partitions.size();
    uint32_t best_set_bits = std::numeric_limits<uint32_t>::max();
    uint32_t best_partition = std::numeric_limits<uint32_t>::max();
    auto& compatible_vertices = memory.k_compatible_vertices[depth];  // fetch current compatible vertices

    // Find the best partition to work with
    for (uint32_t partition = 0; partition < k; ++partition)
    {
        auto num_set_bits = compatible_vertices[partition].count();
        if (!memory.partition_bits[partition] && (num_set_bits < best_set_bits))
        {
            best_set_bits = num_set_bits;
            best_partition = partition;
        }
    }

    // Iterate through compatible vertices in the best partition
    uint32_t adjacent_index = compatible_vertices[best_partition].find_first();
    while (adjacent_index < compatible_vertices[best_partition].size())
    {
        uint32_t vertex = partitions[best_partition][adjacent_index];
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
            // Important to set next to cur, before the update.
            // The following line does not allocate/deallocate because the sizes are exactly the same.
            compatible_vertices_next = compatible_vertices;
            uint32_t offset = 0;
            for (uint32_t partition = 0; partition < k; ++partition)
            {
                auto partition_size = compatible_vertices_next[partition].size();
                if (!memory.partition_bits[partition])
                {
                    for (uint32_t index = 0; index < partition_size; ++index)
                    {
                        compatible_vertices_next[partition][index] &= adjacency_matrix[vertex][index + offset];
                    }
                }
                offset += partition_size;
            }

            memory.partition_bits[best_partition] = 1;

            uint32_t possible_additions = 0;
            for (uint32_t partition = 0; partition < k; ++partition)
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

mimir::generator<const std::vector<uint32_t>&> create_k_clique_in_k_partite_graph_generator(const std::vector<boost::dynamic_bitset<>>& adjacency_matrix,
                                                                                            const std::vector<std::vector<uint32_t>>& partitions,
                                                                                            KPKCWorkspace* memory)
{
    /* Get and verify or create memory layout. */
    auto managed_memory = std::unique_ptr<KPKCWorkspace> {};
    if (!memory)
    {
        // If no external data is provided, manage data internally
        managed_memory = std::make_unique<KPKCWorkspace>(partitions);
        memory = managed_memory.get();
    }

    /* Initialize the memory. */
    memory->initialize_memory(partitions);

    /* Compute k-cliques. */
    for (const auto& result : find_all_k_cliques_in_k_partite_graph_helper(adjacency_matrix, partitions, *memory, 0))
    {
        co_yield result;
    }
}

}
