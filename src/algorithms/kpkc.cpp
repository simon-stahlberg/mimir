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

#include "mimir/algorithms/unique_memory_pool.hpp"
#include "mimir/common/collections.hpp"

#include <functional>
#include <iostream>
#include <limits>
#include <vector>

namespace mimir
{

static thread_local std::vector<uint32_t> s_solution;

static thread_local UniqueMemoryPool<boost::dynamic_bitset<>> s_bitset_pool;
using BitsetUniquePtr = UniqueMemoryPoolPtr<boost::dynamic_bitset<>>;

static thread_local UniqueMemoryPool<std::vector<BitsetUniquePtr>> s_bitset_list_pool;
using BitsetListUniquePtr = UniqueMemoryPoolPtr<std::vector<BitsetUniquePtr>>;

static thread_local UniqueMemoryPool<std::vector<BitsetListUniquePtr>> s_bitset_list_list_pool;
using BitsetMatrixUniquePtr = UniqueMemoryPoolPtr<std::vector<BitsetListUniquePtr>>;

mimir::generator<const std::vector<uint32_t>&> find_all_k_cliques_in_k_partite_graph_helper(const std::vector<boost::dynamic_bitset<>>& adjacency_matrix,
                                                                                            const std::vector<std::vector<uint32_t>>& partitions,
                                                                                            std::vector<uint32_t>& partial_solution,
                                                                                            BitsetUniquePtr& partition_bits,
                                                                                            BitsetMatrixUniquePtr& k_compatible_vertices,
                                                                                            uint32_t depth)
{
    assert(depth < partitions.size());

    uint32_t k = partitions.size();
    uint32_t best_set_bits = std::numeric_limits<uint32_t>::max();
    uint32_t best_partition = std::numeric_limits<uint32_t>::max();

    assert(is_within_bounds(*k_compatible_vertices, depth));
    auto& compatible_vertices = (*k_compatible_vertices)[depth];  // fetch current compatible vertices

    // Find the best partition to work with
    for (uint32_t partition = 0; partition < k; ++partition)
    {
        assert(is_within_bounds(*compatible_vertices, partition));
        auto num_set_bits = (*compatible_vertices)[partition]->count();
        if (!(*partition_bits)[partition] && (num_set_bits < best_set_bits))
        {
            best_set_bits = num_set_bits;
            best_partition = partition;
        }
    }

    // Iterate through compatible vertices in the best partition
    uint32_t adjacent_index = (*compatible_vertices)[best_partition]->find_first();
    while (adjacent_index < (*compatible_vertices)[best_partition]->size())
    {
        assert(is_within_bounds(partitions, best_partition) && is_within_bounds(partitions[best_partition], adjacent_index));
        uint32_t vertex = partitions[best_partition][adjacent_index];
        assert(is_within_bounds(*compatible_vertices, best_partition) && is_within_bounds(*(*compatible_vertices)[best_partition], adjacent_index));
        (*(*compatible_vertices)[best_partition])[adjacent_index] = 0;
        partial_solution.push_back(vertex);

        if (partial_solution.size() == k)
        {
            co_yield partial_solution;
        }
        else
        {
            // Update compatible vertices for the next recursion
            assert(is_within_bounds(*k_compatible_vertices, depth + 1));
            auto& compatible_vertices_next = (*k_compatible_vertices)[depth + 1];
            for (uint32_t partition = 0; partition < k; ++partition)
            {
                assert(is_within_bounds(*compatible_vertices_next, partition));
                assert(is_within_bounds(*compatible_vertices, partition));
                *(*compatible_vertices_next)[partition] = *(*compatible_vertices)[partition];
            }

            uint32_t offset = 0;
            for (uint32_t partition = 0; partition < k; ++partition)
            {
                auto partition_size = (*compatible_vertices_next)[partition]->size();
                if (!(*partition_bits)[partition])
                {
                    for (uint32_t index = 0; index < partition_size; ++index)
                    {
                        (*(*compatible_vertices_next)[partition])[index] &= adjacency_matrix[vertex][index + offset];
                    }
                }
                offset += partition_size;
            }

            (*partition_bits)[best_partition] = 1;

            uint32_t possible_additions = 0;
            for (uint32_t partition = 0; partition < k; ++partition)
            {
                if (!(*partition_bits)[partition] && (*compatible_vertices)[partition]->any())
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

            (*partition_bits)[best_partition] = 0;
        }

        partial_solution.pop_back();
        adjacent_index = (*compatible_vertices)[best_partition]->find_next(adjacent_index);
    }
}

mimir::generator<const std::vector<uint32_t>&> create_k_clique_in_k_partite_graph_generator(const std::vector<boost::dynamic_bitset<>>& adjacency_matrix,
                                                                                            const std::vector<std::vector<uint32_t>>& partitions)
{
    const auto k = partitions.size();

    s_solution.clear();

    auto partition_bits = s_bitset_pool.get_or_allocate();
    partition_bits->resize(k, 0);
    partition_bits->reset();

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

            bitset->resize(partitions[k2].size());

            (*(*k_compatible_vertices)[k1])[k2] = std::move(bitset);
        }
    }

    for (uint32_t k1 = 0; k1 < k; ++k1)
    {
        (*k_compatible_vertices->front())[k1]->set();
    }

    /* Compute k-cliques. */
    for (const auto& result : find_all_k_cliques_in_k_partite_graph_helper(adjacency_matrix, partitions, s_solution, partition_bits, k_compatible_vertices, 0))
    {
        co_yield result;
    }
}

}
