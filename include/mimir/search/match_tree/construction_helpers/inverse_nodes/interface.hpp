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

#ifndef MIMIR_SEARCH_MATCH_TREE_CONSTRUCTION_HELPERS_INVERSE_NODES_INTERFACE_HPP_
#define MIMIR_SEARCH_MATCH_TREE_CONSTRUCTION_HELPERS_INVERSE_NODES_INTERFACE_HPP_

#include "mimir/search/match_tree/declarations.hpp"

namespace mimir::match_tree
{
template<HasConjunctiveCondition Element>
class IInverseNodeVisitor
{
public:
    virtual void accept(const InverseAtomSelectorNode<Element, Fluent>& atom) = 0;
    virtual void accept(const InverseAtomSelectorNode<Element, Derived>& atom) = 0;
    virtual void accept(const InverseNumericConstraintSelectorNode<Element>& constraint) = 0;
    virtual void accept(const InverseElementGeneratorNode<Element>& generator) = 0;
};

/// @brief `Node` implements the interface of nodes.
template<HasConjunctiveCondition Element>
class IInverseNode
{
public:
    virtual ~IInverseNode() = default;

    virtual const InverseNode<Element>& get_parent() const = 0;
    virtual double get_queue_score() const = 0;

    virtual void visit(IInverseNodeVisitor<Element>& visitor) const = 0;
};
}

#endif
