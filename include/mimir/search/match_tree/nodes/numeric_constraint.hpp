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

#ifndef MIMIR_SEARCH_MATCH_TREE_NODES_NUMERIC_CONSTRAINT_HPP_
#define MIMIR_SEARCH_MATCH_TREE_NODES_NUMERIC_CONSTRAINT_HPP_

#include "mimir/search/match_tree/nodes/interface.hpp"

namespace mimir::match_tree
{
template<HasConjunctiveCondition Element>
class NumericConstraintSelectorNode : public INode<Element>
{
private:
    Node<Element> m_true_succ;
    Node<Element> m_dontcare_succ;

    GroundNumericConstraint m_constraint;  ///< we could get rid of it but it can be nice visualization :)

public:
    NumericConstraintSelectorNode(Node<Element>&& true_succ, Node<Element>&& dontcare_succ, GroundNumericConstraint constraint) :
        m_true_succ(std::move(true_succ)),
        m_dontcare_succ(std::move(dontcare_succ)),
        m_constraint(constraint)
    {
    }

    void
    generate_applicable_actions(const DenseState& state, std::vector<const INode<Element>*>& ref_applicable_nodes, std::vector<const Element*>&) const override
    {
        assert(m_true_succ.get());  // The node should not have been created if there were no true successors.
        if (evaluate(m_constraint, state.get_numeric_variables()))
        {
            ref_applicable_nodes.push_back(m_true_succ.get());
        }

        if (m_dontcare_succ.get())
            ref_applicable_nodes.push_back(m_dontcare_succ.get());
    }
};
}

#endif
