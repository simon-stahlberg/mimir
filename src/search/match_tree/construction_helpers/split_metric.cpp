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

namespace mimir::match_tree
{

std::unordered_map<SplitMetricEnum, std::string> split_metric_enum_to_string = {
    { SplitMetricEnum::GINI, "Gini" },
};
extern const std::string& to_string(SplitMetricEnum split_metric) { return split_metric_enum_to_string.at(split_metric); }

}
