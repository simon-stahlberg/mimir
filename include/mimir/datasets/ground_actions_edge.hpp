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

#ifndef MIMIR_DATASETS_GROUND_ACTIONS_EDGE_HPP_
#define MIMIR_DATASETS_GROUND_ACTIONS_EDGE_HPP_

#include "mimir/datasets/declarations.hpp"
#include "mimir/graphs/graph_edges.hpp"
#include "mimir/search/action.hpp"
#include "mimir/search/state.hpp"

#include <span>
#include <vector>

namespace mimir
{

/**
 * Declarations
 */

struct GroundActionsEdgeTag
{
};

using GroundActionsEdge = Edge<GroundActionsEdgeTag, std::span<const GroundAction>>;
using GroundActionsEdgeList = std::vector<GroundActionsEdge>;

inline std::span<const GroundAction> get_actions(const GroundActionsEdge& edge) { return edge.get_property<0>(); }

inline EdgeCost get_cost(const GroundActionsEdge& edge)
{
    auto cost = std::numeric_limits<EdgeCost>::infinity();

    const auto actions = get_actions(edge);
    std::for_each(actions.begin(), actions.end(), [&cost](const auto& action) { cost = std::min(cost, action.get_cost()); });

    return cost;
}

inline GroundAction get_representative_action(const GroundActionsEdge& edge)
{
    assert(!edge.get_property<0>().empty());
    return edge.get_property<0>().front();
}

}

#endif