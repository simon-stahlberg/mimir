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

#ifndef MIMIR_DATASETS_KNOWLEDGE_BASE_OPTIONS_HPP_
#define MIMIR_DATASETS_KNOWLEDGE_BASE_OPTIONS_HPP_

#include "mimir/datasets/generalized_state_space/options.hpp"
#include "mimir/datasets/state_space/options.hpp"
#include "mimir/datasets/tuple_graph/options.hpp"

#include <optional>

namespace mimir::datasets::knowledge_base
{
struct Options
{
    state_space::Options state_space_options;

    std::optional<generalized_state_space::Options> generalized_state_space_options;

    std::optional<tuple_graph::Options> tuple_graph_options;

    Options() : state_space_options(), tuple_graph_options(std::nullopt) {}
    Options(const state_space::Options& state_space_options,
            const std::optional<generalized_state_space::Options>& generalized_state_space_options = std::nullopt,
            const std::optional<tuple_graph::Options>& tuple_graph_options = std::nullopt) :
        state_space_options(state_space_options),
        generalized_state_space_options(generalized_state_space_options),
        tuple_graph_options(tuple_graph_options)
    {
    }
};
}

#endif