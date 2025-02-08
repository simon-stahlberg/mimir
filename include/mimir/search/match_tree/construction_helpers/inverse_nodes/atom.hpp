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

#ifndef MIMIR_SEARCH_MATCH_TREE_CONSTRUCTION_HELPERS_INVERSE_NODES_ATOM_HPP_
#define MIMIR_SEARCH_MATCH_TREE_CONSTRUCTION_HELPERS_INVERSE_NODES_ATOM_HPP_

#include "mimir/formalism/ground_atom.hpp"
#include "mimir/search/match_tree/construction_helpers/inverse_nodes/interface.hpp"

namespace mimir::match_tree
{
template<HasConjunctiveCondition Element, DynamicPredicateTag P>
class InverseAtomSelectorNode : public IInverseNode<Element>
{
private:
    // Meta data
    InverseNode<Element> m_parent;
    double m_queue_score;
    GroundAtom<P> m_atom;

    // Candidates for further refinement.
    std::span<const Element*> m_true_elements;
    std::span<const Element*> m_false_elements;
    std::span<const Element*> m_dontcare_elements;

public:
    explicit InverseAtomSelectorNode(InverseNode<Element> parent,
                                     double queue_score,
                                     GroundAtom<P> atom,
                                     std::span<const Element*> true_elements,
                                     std::span<const Element*> false_elements,
                                     std::span<const Element*> dontcare_elements) :
        m_parent(parent),
        m_queue_score(queue_score),
        m_atom(atom),
        m_true_elements(true_elements),
        m_false_elements(false_elements),
        m_dontcare_elements(dontcare_elements)
    {
    }

    void visit(IInverseNodeVisitor<Element>& visitor) const override { visitor.accept(*this); }

    const InverseNode<Element>& get_parent() const override { return m_parent; }
    double get_queue_score() const override { return m_queue_score; }

    GroundAtom<P> get_atom() const { return m_atom; };
    std::span<const Element*> get_true_elements() const { return m_true_elements; }
    std::span<const Element*> get_false_elements() const { return m_false_elements; }
    std::span<const Element*> get_dontcare_elements() const { return m_dontcare_elements; }
};
}

#endif
