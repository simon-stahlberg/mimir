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

#include "mimir/common/printers.hpp"
#include "mimir/search/declarations.hpp"
#include "mimir/search/match_tree/declarations.hpp"

namespace mimir::match_tree
{
template<HasConjunctiveCondition Element>
class INodeVisitor
{
public:
    virtual void accept(const AtomSelectorNode_TFX<Element, Fluent>& atom) = 0;
    virtual void accept(const AtomSelectorNode_TF<Element, Fluent>& atom) = 0;
    virtual void accept(const AtomSelectorNode_TX<Element, Fluent>& atom) = 0;
    virtual void accept(const AtomSelectorNode_FX<Element, Fluent>& atom) = 0;
    virtual void accept(const AtomSelectorNode_T<Element, Fluent>& atom) = 0;
    virtual void accept(const AtomSelectorNode_F<Element, Fluent>& atom) = 0;
    virtual void accept(const AtomSelectorNode_TFX<Element, Derived>& atom) = 0;
    virtual void accept(const AtomSelectorNode_TF<Element, Derived>& atom) = 0;
    virtual void accept(const AtomSelectorNode_TX<Element, Derived>& atom) = 0;
    virtual void accept(const AtomSelectorNode_FX<Element, Derived>& atom) = 0;
    virtual void accept(const AtomSelectorNode_T<Element, Derived>& atom) = 0;
    virtual void accept(const AtomSelectorNode_F<Element, Derived>& atom) = 0;
    virtual void accept(const NumericConstraintSelectorNode_T<Element>& constraint) = 0;
    virtual void accept(const NumericConstraintSelectorNode_TX<Element>& constraint) = 0;
    virtual void accept(const ElementGeneratorNode_Perfect<Element>& generator) = 0;
    virtual void accept(const ElementGeneratorNode_Imperfect<Element>& generator) = 0;
};

/// @brief `Node` implements the interface of nodes.
template<HasConjunctiveCondition Element>
class INode
{
public:
    virtual ~INode() = default;

    virtual void generate_applicable_actions(const DenseState& state,
                                             std::vector<const INode<Element>*>& ref_applicable_nodes,
                                             std::vector<const Element*>& ref_applicable_elements) const = 0;

    virtual void visit(INodeVisitor<Element>& visitor) const = 0;
};

template<HasConjunctiveCondition Element>
extern std::ostream& operator<<(std::ostream& out, const std::tuple<const Node<Element>&, DotPrinterTag>& tree);
}

#endif
