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

namespace mimir::search::match_tree
{
template<formalism::HasConjunctiveCondition E>
class INodeVisitor
{
public:
    virtual void accept(const AtomSelectorNode_TFX<E, formalism::FluentTag>& atom) = 0;
    virtual void accept(const AtomSelectorNode_TF<E, formalism::FluentTag>& atom) = 0;
    virtual void accept(const AtomSelectorNode_TX<E, formalism::FluentTag>& atom) = 0;
    virtual void accept(const AtomSelectorNode_FX<E, formalism::FluentTag>& atom) = 0;
    virtual void accept(const AtomSelectorNode_T<E, formalism::FluentTag>& atom) = 0;
    virtual void accept(const AtomSelectorNode_F<E, formalism::FluentTag>& atom) = 0;
    virtual void accept(const AtomSelectorNode_TFX<E, formalism::DerivedTag>& atom) = 0;
    virtual void accept(const AtomSelectorNode_TF<E, formalism::DerivedTag>& atom) = 0;
    virtual void accept(const AtomSelectorNode_TX<E, formalism::DerivedTag>& atom) = 0;
    virtual void accept(const AtomSelectorNode_FX<E, formalism::DerivedTag>& atom) = 0;
    virtual void accept(const AtomSelectorNode_T<E, formalism::DerivedTag>& atom) = 0;
    virtual void accept(const AtomSelectorNode_F<E, formalism::DerivedTag>& atom) = 0;
    virtual void accept(const NumericConstraintSelectorNode_T<E>& constraint) = 0;
    virtual void accept(const NumericConstraintSelectorNode_TX<E>& constraint) = 0;
    virtual void accept(const ElementGeneratorNode_Perfect<E>& generator) = 0;
    virtual void accept(const ElementGeneratorNode_Imperfect<E>& generator) = 0;
};

/// @brief `Node` implements the interface of nodes.
template<formalism::HasConjunctiveCondition E>
class INode
{
public:
    virtual ~INode() = default;

    virtual void generate_applicable_actions(const DenseState& state,
                                             const formalism::ProblemImpl& problem,
                                             std::vector<const INode<E>*>& ref_applicable_nodes,
                                             std::vector<const E*>& ref_applicable_elements) const = 0;

    virtual void visit(INodeVisitor<E>& visitor) const = 0;
};

template<formalism::HasConjunctiveCondition E>
extern std::ostream& operator<<(std::ostream& out, const std::tuple<const Node<E>&, DotPrinterTag>& tree);
}

#endif
