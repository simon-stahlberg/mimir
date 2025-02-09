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

#include <concepts>
#include <variant>
#include <vector>

namespace mimir::match_tree
{
enum class SplitMetricEnum
{
    GINI = 0
};

extern std::unordered_map<SplitMetricEnum, std::string> split_metric_enum_to_string;
extern const std::string& to_string(SplitMetricEnum split_metric);

inline double compute_gini_score(const std::vector<size_t>& distribution)
{
    size_t total = 0;
    for (size_t count : distribution)
    {
        total += count;
    }

    if (total == 0)
    {
        return 0.0;
    }

    double gini = 1.0;
    for (size_t count : distribution)
    {
        double probability = static_cast<double>(count) / total;
        gini -= probability * probability;
    }

    return gini;
}

inline double compute_score(const AtomSplitDistribution& distribution, SplitMetricEnum type)
{
    auto packed_distribution =
        std::vector<size_t> { distribution.m_num_true_elements, distribution.m_num_false_elements, distribution.m_num_dont_care_elements };

    switch (type)
    {
        case SplitMetricEnum::GINI:
        {
            return compute_gini_score(packed_distribution);
        }
    }
    return 0.0;
}

inline double compute_score(const NumericConstraintSplitDistribution& distribution, SplitMetricEnum type)
{
    auto packed_distribution = std::vector<size_t> { distribution.m_num_true_elements, distribution.m_num_dont_care_elements };

    switch (type)
    {
        case SplitMetricEnum::GINI:
        {
            return compute_gini_score(packed_distribution);
        }
    }
    return 0.0;
}
}

#endif
