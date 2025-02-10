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

#ifndef MIMIR_SEARCH_MATCH_TREE_NODES_ATOM_HPP_
#define MIMIR_SEARCH_MATCH_TREE_NODES_ATOM_HPP_

#include "mimir/search/match_tree/nodes/interface.hpp"

namespace mimir::match_tree
{
template<HasConjunctiveCondition Element, DynamicPredicateTag P>
class AtomSelectorNode : public INode<Element>
{
private:
    Node<Element> m_true_succ;
    Node<Element> m_false_succ;
    Node<Element> m_dontcare_succ;

    GroundAtom<P> m_atom;  ///< we could get rid of it but it can be nice visualization :)

public:
    explicit AtomSelectorNode(Node<Element>&& true_succ, Node<Element>&& false_succ, Node<Element>&& dontcare_succ, GroundAtom<P> atom) :
        m_true_succ(std::move(true_succ)),
        m_false_succ(std::move(false_succ)),
        m_dontcare_succ(std::move(dontcare_succ)),
        m_atom(atom)
    {
    }

    void
    generate_applicable_actions(const DenseState& state, std::vector<const INode<Element>*>& ref_applicable_nodes, std::vector<const Element*>&) const override
    {
        (state.get_atoms<P>().get(m_atom->get_index())) ? ref_applicable_nodes.push_back(m_true_succ.get()) :
                                                          ref_applicable_nodes.push_back(m_false_succ.get());
        ref_applicable_nodes.push_back(m_dontcare_succ.get());
    }
};
}

#endif
