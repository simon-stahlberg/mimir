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

#include <boost/coroutine2/all.hpp>
#include <boost/dynamic_bitset.hpp>
#include <vector>

namespace mimir
{

using clique_coroutine_t = boost::coroutines2::coroutine<std::vector<size_t>>;

// Find all cliques of size k in a k-partite graph
clique_coroutine_t::pull_type create_k_clique_in_k_partite_graph_generator(const std::vector<boost::dynamic_bitset<>>& adjacency_matrix,
                                                                           const std::vector<std::vector<size_t>>& partitions);

}

#endif  // MIMIR_ALGORITHMS_KPKC_HPP_
