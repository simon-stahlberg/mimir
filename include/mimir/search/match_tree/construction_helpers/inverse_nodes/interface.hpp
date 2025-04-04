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

namespace mimir::search::match_tree
{
template<formalism::HasConjunctiveCondition E>
class IInverseNodeVisitor
{
public:
    virtual void accept(const InverseAtomSelectorNode_TFX<E, formalism::FluentTag>& atom) = 0;
    virtual void accept(const InverseAtomSelectorNode_TF<E, formalism::FluentTag>& atom) = 0;
    virtual void accept(const InverseAtomSelectorNode_TX<E, formalism::FluentTag>& atom) = 0;
    virtual void accept(const InverseAtomSelectorNode_FX<E, formalism::FluentTag>& atom) = 0;
    virtual void accept(const InverseAtomSelectorNode_T<E, formalism::FluentTag>& atom) = 0;
    virtual void accept(const InverseAtomSelectorNode_F<E, formalism::FluentTag>& atom) = 0;
    virtual void accept(const InverseAtomSelectorNode_TFX<E, formalism::DerivedTag>& atom) = 0;
    virtual void accept(const InverseAtomSelectorNode_TF<E, formalism::DerivedTag>& atom) = 0;
    virtual void accept(const InverseAtomSelectorNode_TX<E, formalism::DerivedTag>& atom) = 0;
    virtual void accept(const InverseAtomSelectorNode_FX<E, formalism::DerivedTag>& atom) = 0;
    virtual void accept(const InverseAtomSelectorNode_T<E, formalism::DerivedTag>& atom) = 0;
    virtual void accept(const InverseAtomSelectorNode_F<E, formalism::DerivedTag>& atom) = 0;
    virtual void accept(const InverseNumericConstraintSelectorNode_TX<E>& constraint) = 0;
    virtual void accept(const InverseNumericConstraintSelectorNode_T<E>& constraint) = 0;
    virtual void accept(const InverseElementGeneratorNode_Perfect<E>& generator) = 0;
    virtual void accept(const InverseElementGeneratorNode_Imperfect<E>& generator) = 0;
};

/// @brief `Node` implements the interface of nodes.
template<formalism::HasConjunctiveCondition E>
class IInverseNode
{
protected:
    const IInverseNode<E>* m_parent;  ///< Raw pointer to parent node, we must ensure that root is kept alive!
    SplitList m_useless_splits;       ///< Splits that were detected as useless.
    size_t m_root_distance;           ///< The distance to the root node.

public:
    IInverseNode(const IInverseNode<E>* parent, SplitList useless_splits) : m_parent(parent), m_useless_splits(std::move(useless_splits)) {}
    virtual ~IInverseNode() = default;

    virtual const IInverseNode<E>* get_parent() const { return m_parent; }
    virtual const SplitList& get_useless_splits() const { return m_useless_splits; }

    virtual void visit(IInverseNodeVisitor<E>& visitor) const = 0;
};

template<formalism::HasConjunctiveCondition E>
extern std::ostream& operator<<(std::ostream& out, const std::tuple<const InverseNode<E>&, DotPrinterTag>& tree);
}

#endif
