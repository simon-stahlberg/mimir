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

#include "mimir/common/printers.hpp"
#include "mimir/search/match_tree/construction_helpers/split.hpp"
#include "mimir/search/match_tree/declarations.hpp"

#include <map>
#include <unordered_map>

namespace mimir::match_tree
{
template<HasConjunctiveCondition Element>
class IInverseNodeVisitor
{
public:
    virtual void accept(const InverseAtomSelectorNode_TFX<Element, Fluent>& atom) = 0;
    virtual void accept(const InverseAtomSelectorNode_TF<Element, Fluent>& atom) = 0;
    virtual void accept(const InverseAtomSelectorNode_TX<Element, Fluent>& atom) = 0;
    virtual void accept(const InverseAtomSelectorNode_FX<Element, Fluent>& atom) = 0;
    virtual void accept(const InverseAtomSelectorNode_T<Element, Fluent>& atom) = 0;
    virtual void accept(const InverseAtomSelectorNode_F<Element, Fluent>& atom) = 0;
    virtual void accept(const InverseAtomSelectorNode_TFX<Element, Derived>& atom) = 0;
    virtual void accept(const InverseAtomSelectorNode_TF<Element, Derived>& atom) = 0;
    virtual void accept(const InverseAtomSelectorNode_TX<Element, Derived>& atom) = 0;
    virtual void accept(const InverseAtomSelectorNode_FX<Element, Derived>& atom) = 0;
    virtual void accept(const InverseAtomSelectorNode_T<Element, Derived>& atom) = 0;
    virtual void accept(const InverseAtomSelectorNode_F<Element, Derived>& atom) = 0;
    virtual void accept(const InverseNumericConstraintSelectorNode_TX<Element>& constraint) = 0;
    virtual void accept(const InverseNumericConstraintSelectorNode_T<Element>& constraint) = 0;
    virtual void accept(const InverseElementGeneratorNode_Perfect<Element>& generator) = 0;
    virtual void accept(const InverseElementGeneratorNode_Imperfect<Element>& generator) = 0;
};

/// @brief `Node` implements the interface of nodes.
template<HasConjunctiveCondition Element>
class IInverseNode
{
protected:
    const IInverseNode<Element>* m_parent;  ///< Raw pointer to parent node, we must ensure that root is kept alive!
    SplitList m_useless_splits;             ///< Splits that were detected as useless.
    size_t m_root_distance;                 ///< The distance to the root node.

public:
    IInverseNode(const IInverseNode<Element>* parent, SplitList useless_splits) : m_parent(parent), m_useless_splits(std::move(useless_splits)) {}
    virtual ~IInverseNode() = default;

    virtual const IInverseNode<Element>* get_parent() const { return m_parent; }
    virtual const SplitList& get_useless_splits() const { return m_useless_splits; }

    virtual void visit(IInverseNodeVisitor<Element>& visitor) const = 0;
};

template<HasConjunctiveCondition Element>
extern std::ostream& operator<<(std::ostream& out, const std::tuple<const InverseNode<Element>&, DotPrinterTag>& tree);
}

#endif
