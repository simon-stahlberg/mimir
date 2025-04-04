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

#ifndef MIMIR_SEARCH_MATCH_TREE_CONSTRUCTION_HELPERS_NODE_CREATION_HPP_
#define MIMIR_SEARCH_MATCH_TREE_CONSTRUCTION_HELPERS_NODE_CREATION_HPP_

#include "mimir/search/match_tree/declarations.hpp"

#include <deque>
#include <vector>

namespace mimir::search::match_tree
{
template<formalism::HasConjunctiveCondition E>
extern Node<E> parse_inverse_tree_iteratively(const InverseNode<E>& root);

template<formalism::HasConjunctiveCondition E>
extern Node<E> create_root_generator_node(std::span<const E*> elements);
}

#endif
