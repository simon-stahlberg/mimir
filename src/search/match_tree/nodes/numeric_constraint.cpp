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

#include "mimir/search/match_tree/nodes/numeric_constraint.hpp"

#include "mimir/formalism/ground_action.hpp"
#include "mimir/formalism/ground_axiom.hpp"
#include "mimir/formalism/ground_numeric_constraint.hpp"
#include "mimir/search/dense_state.hpp"

namespace mimir::match_tree
{
template<HasConjunctiveCondition Element>
void NumericConstraintSelectorNode_T<Element>::visit_impl(INodeVisitor<Element>& visitor) const
{
    visitor.accept(*this);
}

template<HasConjunctiveCondition Element>
NumericConstraintSelectorNode_T<Element>::NumericConstraintSelectorNode_T(Node<Element>&& true_child, GroundNumericConstraint constraint) :
    NumericConstraintSelectorNodeBase<NumericConstraintSelectorNode_T<Element>, Element>(constraint),
    m_true_child(std::move(true_child))
{
    assert(m_true_child);
}

template<HasConjunctiveCondition Element>
void NumericConstraintSelectorNode_T<Element>::generate_applicable_actions(const DenseState& state,
                                                                           std::vector<const INode<Element>*>& ref_applicable_nodes,
                                                                           std::vector<const Element*>&) const
{
    if (evaluate(this->m_constraint, state.get_numeric_variables()))
    {
        ref_applicable_nodes.push_back(m_true_child.get());
    }
}

template<HasConjunctiveCondition Element>
const Node<Element>& NumericConstraintSelectorNode_T<Element>::get_true_child() const
{
    return m_true_child;
};

template class NumericConstraintSelectorNode_T<GroundActionImpl>;
template class NumericConstraintSelectorNode_T<GroundAxiomImpl>;

template<HasConjunctiveCondition Element>
void NumericConstraintSelectorNode_TX<Element>::visit_impl(INodeVisitor<Element>& visitor) const
{
    visitor.accept(*this);
}

template<HasConjunctiveCondition Element>
NumericConstraintSelectorNode_TX<Element>::NumericConstraintSelectorNode_TX(Node<Element>&& true_child,
                                                                            Node<Element>&& dontcare_child,
                                                                            GroundNumericConstraint constraint) :
    NumericConstraintSelectorNodeBase<NumericConstraintSelectorNode_TX<Element>, Element>(constraint),
    m_true_child(std::move(true_child)),
    m_dontcare_child(std::move(dontcare_child))
{
    assert(m_true_child);
    assert(m_dontcare_child);
}

template<HasConjunctiveCondition Element>
void NumericConstraintSelectorNode_TX<Element>::generate_applicable_actions(const DenseState& state,
                                                                            std::vector<const INode<Element>*>& ref_applicable_nodes,
                                                                            std::vector<const Element*>&) const
{
    ref_applicable_nodes.push_back(m_dontcare_child.get());

    if (evaluate(this->m_constraint, state.get_numeric_variables()))
    {
        ref_applicable_nodes.push_back(m_true_child.get());
    }
}

template<HasConjunctiveCondition Element>
const Node<Element>& NumericConstraintSelectorNode_TX<Element>::get_true_child() const
{
    return m_true_child;
};

template<HasConjunctiveCondition Element>
const Node<Element>& NumericConstraintSelectorNode_TX<Element>::get_dontcare_child() const
{
    return m_dontcare_child;
}

template class NumericConstraintSelectorNode_TX<GroundActionImpl>;
template class NumericConstraintSelectorNode_TX<GroundAxiomImpl>;

}
