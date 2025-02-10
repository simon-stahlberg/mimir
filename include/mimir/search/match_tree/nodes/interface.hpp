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

#ifndef MIMIR_SEARCH_MATCH_TREE_NODES_INTERFACE_HPP_
#define MIMIR_SEARCH_MATCH_TREE_NODES_INTERFACE_HPP_

#include "mimir/search/dense_state.hpp"
#include "mimir/search/match_tree/declarations.hpp"

namespace mimir::match_tree
{
/// @brief `Node` implements the interface of nodes.
template<HasConjunctiveCondition Element>
class INode
{
public:
    virtual ~INode() = default;

    virtual void generate_applicable_actions(const DenseState& state,
                                             std::vector<const INode<Element>*>& ref_applicable_nodes,
                                             std::vector<const Element*>& ref_applicable_elements) const = 0;
};
}

#endif
