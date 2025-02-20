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
template<HasConjunctiveCondition E>
class INodeVisitor
{
public:
    virtual void accept(const AtomSelectorNode_TFX<E, Fluent>& atom) = 0;
    virtual void accept(const AtomSelectorNode_TF<E, Fluent>& atom) = 0;
    virtual void accept(const AtomSelectorNode_TX<E, Fluent>& atom) = 0;
    virtual void accept(const AtomSelectorNode_FX<E, Fluent>& atom) = 0;
    virtual void accept(const AtomSelectorNode_T<E, Fluent>& atom) = 0;
    virtual void accept(const AtomSelectorNode_F<E, Fluent>& atom) = 0;
    virtual void accept(const AtomSelectorNode_TFX<E, Derived>& atom) = 0;
    virtual void accept(const AtomSelectorNode_TF<E, Derived>& atom) = 0;
    virtual void accept(const AtomSelectorNode_TX<E, Derived>& atom) = 0;
    virtual void accept(const AtomSelectorNode_FX<E, Derived>& atom) = 0;
    virtual void accept(const AtomSelectorNode_T<E, Derived>& atom) = 0;
    virtual void accept(const AtomSelectorNode_F<E, Derived>& atom) = 0;
    virtual void accept(const NumericConstraintSelectorNode_T<E>& constraint) = 0;
    virtual void accept(const NumericConstraintSelectorNode_TX<E>& constraint) = 0;
    virtual void accept(const ElementGeneratorNode_Perfect<E>& generator) = 0;
    virtual void accept(const ElementGeneratorNode_Imperfect<E>& generator) = 0;
};

/// @brief `Node` implements the interface of nodes.
template<HasConjunctiveCondition E>
class INode
{
public:
    virtual ~INode() = default;

    virtual void generate_applicable_actions(const DenseState& state,
                                             Problem problem,
                                             std::vector<const INode<E>*>& ref_applicable_nodes,
                                             std::vector<const E*>& ref_applicable_elements) const = 0;

    virtual void visit(INodeVisitor<E>& visitor) const = 0;
};

template<HasConjunctiveCondition E>
extern std::ostream& operator<<(std::ostream& out, const std::tuple<const Node<E>&, DotPrinterTag>& tree);
}

#endif
