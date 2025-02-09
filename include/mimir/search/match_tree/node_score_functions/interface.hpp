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

#ifndef MIMIR_SEARCH_MATCH_TREE_NODE_SCORE_FUNCTIONS_INTERFACE_HPP_
#define MIMIR_SEARCH_MATCH_TREE_NODE_SCORE_FUNCTIONS_INTERFACE_HPP_

#include "mimir/search/match_tree/declarations.hpp"

namespace mimir::match_tree
{

/* Customization point 1: INodeScoreFunction*/

/// @brief `INodeScoringFunction` allows computing a score for a node.
template<HasConjunctiveCondition Element>
class INodeScoreFunction
{
public:
    virtual ~INodeScoreFunction() = default;

    virtual double compute_score(const InverseNode<Element>& node) = 0;
};

}

#endif
