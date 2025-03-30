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

#ifndef MIMIR_SEARCH_MATCH_TREE_CONSTRUCTION_HELPERS_SPLIT_METRICS_HPP_
#define MIMIR_SEARCH_MATCH_TREE_CONSTRUCTION_HELPERS_SPLIT_METRICS_HPP_

#include "mimir/search/match_tree/construction_helpers/split.hpp"
#include "mimir/search/match_tree/node_splitters/interface.hpp"

#include <concepts>
#include <variant>
#include <vector>

namespace mimir::search::match_tree
{
extern std::unordered_map<SplitMetricEnum, std::string> split_metric_enum_to_string;
extern const std::string& to_string(SplitMetricEnum split_metric);

extern std::unordered_map<OptimizationDirectionEnum, std::string> optimization_direction_enum_to_string;
extern const std::string& to_string(OptimizationDirectionEnum direction);

/// @brief Return the worst possible score for the metric type.
extern double worst_score(OptimizationDirectionEnum direction);

/// @brief Return true iff lhs has a better score than rhs for the metric type.
extern bool better_score(double lhs, double rhs, OptimizationDirectionEnum direction);

extern double compute_score(const AtomSplitDistribution& distribution, SplitMetricEnum type);

extern double compute_score(const NumericConstraintSplitDistribution& distribution, SplitMetricEnum type);

extern double compute_score(const Split& split, SplitMetricEnum type);
}

#endif
