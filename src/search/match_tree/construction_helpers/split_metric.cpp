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

#include "mimir/search/match_tree/construction_helpers/split_metrics.hpp"

using namespace mimir::formalism;

namespace mimir::search::match_tree
{

std::unordered_map<SplitMetricEnum, std::string> split_metric_enum_to_string = {
    { SplitMetricEnum::GINI, "Gini" },
    { SplitMetricEnum::FREQUENCY, "MaxFrequency" },
};
const std::string& to_string(SplitMetricEnum split_metric) { return split_metric_enum_to_string.at(split_metric); }

std::unordered_map<OptimizationDirectionEnum, std::string> optimization_direction_enum_to_string = {
    { OptimizationDirectionEnum::MINIMIZE, "minimize" },
    { OptimizationDirectionEnum::MAXIMIZE, "maxmimize" },
};
const std::string& to_string(OptimizationDirectionEnum direction)
{
    {
        return optimization_direction_enum_to_string.at(direction);
    }
}

extern double worst_score(OptimizationDirectionEnum direction)
{
    switch (direction)
    {
        case OptimizationDirectionEnum::MINIMIZE:
        {
            return std::numeric_limits<double>::infinity();
        }
        case OptimizationDirectionEnum::MAXIMIZE:
        {
            return -std::numeric_limits<double>::infinity();
        }
    }

    throw std::runtime_error("worst_score(type): Undefined worst score for direction: " + to_string(direction));
}

bool better_score(double lhs, double rhs, OptimizationDirectionEnum direction)
{
    switch (direction)
    {
        case OptimizationDirectionEnum::MINIMIZE:
        {
            return lhs < rhs;
        }
        case OptimizationDirectionEnum::MAXIMIZE:
        {
            return lhs > rhs;
        }
    }

    throw std::runtime_error("better_score(lhs, rhs, type): Undefined comparison for direction: " + to_string(direction));
}

static double compute_gini_score(const std::vector<size_t>& distribution)
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

double compute_score(const AtomSplitDistribution& distribution, SplitMetricEnum type)
{
    auto packed_distribution = std::vector<size_t> { distribution.num_true_elements, distribution.num_false_elements, distribution.num_dont_care_elements };

    switch (type)
    {
        case SplitMetricEnum::GINI:
        {
            return compute_gini_score(packed_distribution);
        }
        case SplitMetricEnum::FREQUENCY:
        {
            return distribution.num_true_elements + distribution.num_false_elements;
        }
    }

    throw std::runtime_error("compute_score(distribution, type): Undefined score computation of atom distribution for type: " + to_string(type));
}

double compute_score(const NumericConstraintSplitDistribution& distribution, SplitMetricEnum type)
{
    auto packed_distribution = std::vector<size_t> { distribution.num_true_elements, distribution.num_dont_care_elements };

    switch (type)
    {
        case SplitMetricEnum::GINI:
        {
            return compute_gini_score(packed_distribution);
        }
        case SplitMetricEnum::FREQUENCY:
        {
            return distribution.num_true_elements;
        }
    }

    throw std::runtime_error("compute_score(distribution, type): Undefined score computation of numeric constraint distribution for type: " + to_string(type));
}

double compute_score(const Split& split, SplitMetricEnum type)
{
    return std::visit([type](auto&& arg) { return compute_score(arg.distribution, type); }, split);
}

}
