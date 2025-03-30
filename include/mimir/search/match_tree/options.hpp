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

#ifndef MIMIR_SEARCH_MATCH_TREE_OPTIONS_HPP_
#define MIMIR_SEARCH_MATCH_TREE_OPTIONS_HPP_

#include "mimir/search/match_tree/declarations.hpp"
#include "mimir/search/match_tree/node_splitters/interface.hpp"

namespace mimir::search::match_tree
{
struct Options
{
    bool enable_dump_dot_file = false;
    fs::path output_dot_file = "match_tree.dot";
    size_t max_num_nodes = std::numeric_limits<size_t>::max();
    SplitStrategyEnum split_strategy = SplitStrategyEnum::DYNAMIC;
    SplitMetricEnum split_metric = SplitMetricEnum::FREQUENCY;
    OptimizationDirectionEnum optimization_direction = OptimizationDirectionEnum::MAXIMIZE;
};

}

#endif
