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

#ifndef MIMIR_SEARCH_MATCH_TREE_CONSTRUCTION_HELPERS_INVERSE_NODES_PLACEHOLDER_HPP_
#define MIMIR_SEARCH_MATCH_TREE_CONSTRUCTION_HELPERS_INVERSE_NODES_PLACEHOLDER_HPP_

#include "mimir/search/match_tree/declarations.hpp"

namespace mimir::match_tree
{
template<HasConjunctiveCondition Element>
class PlaceholderNodeImpl
{
private:
    const IInverseNode<Element>* m_parent;
    InverseNode<Element>* m_parents_child;
    size_t m_root_distance;
    std::span<const Element*> m_elements;

public:
    PlaceholderNodeImpl(const IInverseNode<Element>* parent, InverseNode<Element>* parents_child, size_t root_distance, std::span<const Element*> elements);
    PlaceholderNodeImpl(const PlaceholderNodeImpl& other) = delete;
    PlaceholderNodeImpl& operator=(const PlaceholderNodeImpl& other) = delete;
    PlaceholderNodeImpl(PlaceholderNodeImpl&& other) = delete;
    PlaceholderNodeImpl& operator=(PlaceholderNodeImpl&& other) = delete;

    const IInverseNode<Element>* get_parent() const;
    InverseNode<Element>& get_parents_child() const;
    size_t get_root_distance() const;
    std::span<const Element*> get_elements() const;
};
}

#endif
