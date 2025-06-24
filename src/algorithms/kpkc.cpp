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

#include "mimir/algorithms/unique_object_pool.hpp"
#include "mimir/common/collections.hpp"

#include <functional>
#include <iostream>
#include <limits>
#include <vector>

namespace mimir
{

/**
 * Heap-managed coroutine variables.
 */

static thread_local UniqueObjectPool<std::vector<uint32_t>> s_solution_pool;
using SolutionPtr = UniqueObjectPoolPtr<std::vector<uint32_t>>;

static thread_local UniqueObjectPool<boost::dynamic_bitset<>> s_bitset_pool;
using BitsetUniquePtr = UniqueObjectPoolPtr<boost::dynamic_bitset<>>;

static thread_local UniqueObjectPool<std::vector<BitsetUniquePtr>> s_bitset_list_pool;
using BitsetListUniquePtr = UniqueObjectPoolPtr<std::vector<BitsetUniquePtr>>;

static thread_local UniqueObjectPool<std::vector<BitsetListUniquePtr>> s_bitset_list_list_pool;
using BitsetMatrixUniquePtr = UniqueObjectPoolPtr<std::vector<BitsetListUniquePtr>>;

/**
 * Recursive call
 */

mimir::generator<const std::vector<uint32_t>&> find_all_k_cliques_in_k_partite_graph_helper(const std::vector<boost::dynamic_bitset<>>& adjacency_matrix,
                                                                                            const std::vector<std::vector<uint32_t>>& partitions,
                                                                                            std::vector<uint32_t>& partial_solution,
                                                                                            boost::dynamic_bitset<>& partition_bits,
                                                                                            std::vector<BitsetListUniquePtr>& k_compatible_vertices,
                                                                                            uint32_t depth)
{
    assert(depth < partitions.size());

    uint32_t k = partitions.size();
    uint32_t best_set_bits = std::numeric_limits<uint32_t>::max();
    uint32_t best_partition = std::numeric_limits<uint32_t>::max();

    assert(is_within_bounds(k_compatible_vertices, depth));
    auto& compatible_vertices = *k_compatible_vertices[depth];

    // Find the best partition to work with
    for (uint32_t partition = 0; partition < k; ++partition)
    {
        assert(is_within_bounds(compatible_vertices, partition));
        auto num_set_bits = compatible_vertices[partition]->count();
        if (!partition_bits[partition] && (num_set_bits < best_set_bits))
        {
            best_set_bits = num_set_bits;
            best_partition = partition;
        }
    }

    // Iterate through compatible vertices in the best partition
    assert(is_within_bounds(compatible_vertices, best_partition));
    auto& best_partition_compatible_vertices = *compatible_vertices[best_partition];
    uint32_t adjacent_index = best_partition_compatible_vertices.find_first();
    while (adjacent_index < best_partition_compatible_vertices.size())
    {
        assert(is_within_bounds(partitions, best_partition) && is_within_bounds(partitions[best_partition], adjacent_index));
        uint32_t vertex = partitions[best_partition][adjacent_index];
        assert(is_within_bounds(compatible_vertices, best_partition) && is_within_bounds(best_partition_compatible_vertices, adjacent_index));
        best_partition_compatible_vertices[adjacent_index] = 0;

        partial_solution.push_back(vertex);

        if (partial_solution.size() == k)
        {
            co_yield partial_solution;
        }
        else
        {
            assert(partial_solution.size() - 1 == depth);

            // Update compatible vertices for the next recursion
            assert(is_within_bounds(k_compatible_vertices, depth + 1));
            auto& compatible_vertices_next = *k_compatible_vertices[depth + 1];
            for (uint32_t partition = 0; partition < k; ++partition)
            {
                assert(is_within_bounds(compatible_vertices_next, partition));
                assert(is_within_bounds(compatible_vertices, partition));
                auto& partition_compatible_vertices_next = *compatible_vertices_next[partition];
                auto& partition_compatible_vertices = *compatible_vertices[partition];
                assert(partition_compatible_vertices_next.size() == partition_compatible_vertices.size());
                partition_compatible_vertices_next = partition_compatible_vertices;  // copy bitsets from current to next iteration
            }

            uint32_t offset = 0;
            for (uint32_t partition = 0; partition < k; ++partition)
            {
                assert(is_within_bounds(compatible_vertices_next, partition));
                auto& partition_compatible_vertices_next = *compatible_vertices_next[partition];
                auto partition_size = partition_compatible_vertices_next.size();
                if (!partition_bits[partition])
                {
                    for (uint32_t index = 0; index < partition_size; ++index)
                    {
                        assert(is_within_bounds(compatible_vertices_next, partition));
                        partition_compatible_vertices_next[index] &= adjacency_matrix[vertex][index + offset];
                    }
                }
                offset += partition_size;
            }

            partition_bits[best_partition] = 1;

            uint32_t possible_additions = 0;
            for (uint32_t partition = 0; partition < k; ++partition)
            {
                assert(is_within_bounds(compatible_vertices, partition));
                auto& partition_compatible_vertices = *compatible_vertices[partition];
                if (!partition_bits[partition] && partition_compatible_vertices.any())
                {
                    ++possible_additions;
                }
            }

            if ((partial_solution.size() + possible_additions) == k)
            {
                for (const auto& result : find_all_k_cliques_in_k_partite_graph_helper(adjacency_matrix,
                                                                                       partitions,
                                                                                       partial_solution,
                                                                                       partition_bits,
                                                                                       k_compatible_vertices,
                                                                                       depth + 1))
                {
                    co_yield result;
                }
            }

            partition_bits[best_partition] = 0;
        }

        partial_solution.pop_back();

        adjacent_index = best_partition_compatible_vertices.find_next(adjacent_index);
    }
}

bool verify_input_dimensions(const std::vector<boost::dynamic_bitset<>>& adjacency_matrix, const std::vector<std::vector<uint32_t>>& partitions)
{
    size_t total_vertices = 0;
    for (const auto& partition : partitions)
        total_vertices += partition.size();

    if (adjacency_matrix.size() != total_vertices)
        return false;

    for (const auto& row : adjacency_matrix)
    {
        if (row.size() != total_vertices)
            return false;
    }

    return true;
}

mimir::generator<const std::vector<uint32_t>&> create_k_clique_in_k_partite_graph_generator(const std::vector<boost::dynamic_bitset<>>& adjacency_matrix,
                                                                                            const std::vector<std::vector<uint32_t>>& partitions)
{
    assert(verify_input_dimensions(adjacency_matrix, partitions));

    const auto k = partitions.size();

    /* Allocate and initialize solution */
    auto solution = s_solution_pool.get_or_allocate();
    solution->clear();

    /* Allocate and initialize partition bits */
    auto partition_bits = s_bitset_pool.get_or_allocate();
    partition_bits->reset();
    partition_bits->resize(k, false);

    /* Allocate and initialize k_compatible_vertices */
    auto k_compatible_vertices = s_bitset_list_list_pool.get_or_allocate();

    k_compatible_vertices->clear();
    k_compatible_vertices->resize(k);

    for (uint32_t k1 = 0; k1 < k; ++k1)
    {
        auto biset_vec = s_bitset_list_pool.get_or_allocate();

        biset_vec->clear();
        biset_vec->resize(k);

        (*k_compatible_vertices)[k1] = std::move(biset_vec);

        for (uint32_t k2 = 0; k2 < k; ++k2)
        {
            auto bitset = s_bitset_pool.get_or_allocate();

            bitset->reset();
            bitset->resize(partitions[k2].size(), false);

            (*(*k_compatible_vertices)[k1])[k2] = std::move(bitset);
        }
    }

    for (uint32_t k1 = 0; k1 < k; ++k1)
    {
        (*k_compatible_vertices->front())[k1]->set();
    }

    /* Enumerate all k-cliques. */
    for (const auto& result : find_all_k_cliques_in_k_partite_graph_helper(adjacency_matrix, partitions, *solution, *partition_bits, *k_compatible_vertices, 0))
    {
        co_yield result;
    }
}

}
