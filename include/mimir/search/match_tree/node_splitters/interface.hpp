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

#ifndef MIMIR_SEARCH_MATCH_TREE_NODE_SPLITTERS_INTERFACE_HPP_
#define MIMIR_SEARCH_MATCH_TREE_NODE_SPLITTERS_INTERFACE_HPP_

#include "mimir/search/match_tree/construction_helpers/split.hpp"
#include "mimir/search/match_tree/declarations.hpp"

#include <queue>

namespace mimir::search::match_tree
{
/* Customization point 2: NodeSplitter */

enum class SplitStrategyEnum
{
    DYNAMIC = 0,
    HYBRID = 1,
    STATIC = 2,
};

enum class SplitMetricEnum
{
    GINI = 0,
    FREQUENCY = 1
};

enum class OptimizationDirectionEnum
{
    MINIMIZE,
    MAXIMIZE
};

/// @brief `INodeSplitter` computes the best split for a given set of elements.
/// @tparam Element
template<formalism::HasConjunctiveCondition E>
class INodeSplitter
{
public:
    virtual ~INodeSplitter() = default;

    virtual std::pair<Node<E>, Statistics> fit(std::span<const E*> elements) = 0;
};

}

#endif
