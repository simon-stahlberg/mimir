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

#include <boost/dynamic_bitset.hpp>
#include <vector>

namespace mimir
{
// Find all cliques of size k in a k-partite graph
void find_all_k_cliques_in_k_partite_graph(const std::vector<boost::dynamic_bitset<>>& adjacency_matrix,
                                           const std::vector<std::vector<size_t>>& partitions,
                                           std::vector<std::vector<std::size_t>>& out_cliques);
}

#endif  // MIMIR_ALGORITHMS_KPKC_HPP_
