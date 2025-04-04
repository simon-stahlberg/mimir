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

#ifndef MIMIR_SEARCH_MATCH_TREE_STATISTICS_HPP_
#define MIMIR_SEARCH_MATCH_TREE_STATISTICS_HPP_

#include "mimir/search/match_tree/declarations.hpp"

#include <chrono>
#include <map>
#include <ostream>
#include <vector>

namespace mimir::search::match_tree
{
struct Statistics
{
    size_t num_elements = 0;
    size_t num_nodes = 1;  ///< Tree always has 1 root node
    std::vector<size_t> generator_distribution = std::vector<size_t>();
    std::vector<size_t> perfect_generator_distribution = std::vector<size_t>();
    std::vector<size_t> imperfect_generator_distribution = std::vector<size_t>();
    std::chrono::time_point<std::chrono::high_resolution_clock> construction_start_time_point = std::chrono::high_resolution_clock::now();
    std::chrono::time_point<std::chrono::high_resolution_clock> construction_end_time_point = std::chrono::high_resolution_clock::now();
};

template<formalism::HasConjunctiveCondition E>
extern void parse_generator_distribution_iteratively(const Node<E>& root, Statistics& ref_statistics);

extern std::ostream& operator<<(std::ostream& os, const Statistics& statistics);
}

#endif
