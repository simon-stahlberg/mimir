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

#include "mimir/formalism/ground_numeric_constraint.hpp"
#include "mimir/search/match_tree/nodes/interface.hpp"

namespace mimir::match_tree
{
template<HasConjunctiveCondition Element>
class NumericConstraintSelectorNode : public INode<Element>
{
private:
    Node<Element> m_true_child;
    Node<Element> m_dontcare_child;

    GroundNumericConstraint m_constraint;

public:
    NumericConstraintSelectorNode(Node<Element>&& true_child, Node<Element>&& dontcare_child, GroundNumericConstraint constraint) :
        m_true_child(std::move(true_child)),
        m_dontcare_child(std::move(dontcare_child)),
        m_constraint(constraint)
    {
        assert(m_constraint);
        assert(m_true_child);
        assert(m_dontcare_child);
    }

    void
    generate_applicable_actions(const DenseState& state, std::vector<const INode<Element>*>& ref_applicable_nodes, std::vector<const Element*>&) const override
    {
        assert(m_true_child.get());  // The node should not have been created if there were no true successors.
        if (evaluate(m_constraint, state.get_numeric_variables()))
        {
            ref_applicable_nodes.push_back(m_true_child.get());
        }

        if (m_dontcare_child.get())
            ref_applicable_nodes.push_back(m_dontcare_child.get());
    }

    const Node<Element>& get_true_child() const { return m_true_child; };
    const Node<Element>& get_dontcare_child() const { return m_dontcare_child; }

    GroundNumericConstraint get_constraint() const { return m_constraint; }

    void visit(INodeVisitor<Element>& visitor) const override { visitor.accept(*this); }
};
}

#endif
