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

#ifndef MIMIR_ALGORITHMS_KPKC_HPP_
#define MIMIR_ALGORITHMS_KPKC_HPP_

#include "mimir/algorithms/generator.hpp"

#include <boost/dynamic_bitset.hpp>
#include <optional>
#include <vector>

namespace mimir
{

/// @brief `KPKCWorkspace` manages preallocated memory to be used with the KPKC algorithm.
///
/// The constructor initializes the correct memory layout.
/// KPKC will verify the memory layout and throw an exception if incorrect.
struct KPKCWorkspace
{
    explicit KPKCWorkspace(const std::vector<std::vector<uint32_t>>& partitions);

    /// @brief Verifies the memory layout to catch accidental modifications.
    /// @param partitions
    void verify_memory_layout(const std::vector<std::vector<uint32_t>>& partitions);

    /// @brief Initializes the memory.
    /// @param partitions
    void initialize_memory(const std::vector<std::vector<uint32_t>>& partitions);

    boost::dynamic_bitset<> partition_bits;
    std::vector<uint32_t> partial_solution;
    std::vector<std::vector<boost::dynamic_bitset<>>> k_compatible_vertices;
};

/// @brief Find all cliques of size k in a k-partite graph.
/// @param adjacency_matrix is the adjacency matrix.
/// @param partitions is the partitioning.
/// @param memory preallocated memory, if not given, the algorithm will perform O(k^2) many allocations.
/// @return a generator to enumerate all k-cliques.
mimir::generator<const std::vector<uint32_t>&> create_k_clique_in_k_partite_graph_generator(const std::vector<boost::dynamic_bitset<>>& adjacency_matrix,
                                                                                            const std::vector<std::vector<uint32_t>>& partitions,
                                                                                            KPKCWorkspace* memory = nullptr);

}

#endif  // MIMIR_ALGORITHMS_KPKC_HPP_
