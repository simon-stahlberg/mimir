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
#include "mimir/formalism/problem.hpp"
#include "mimir/search/dense_state.hpp"

using namespace mimir::formalism;

namespace mimir::search::match_tree
{
template<formalism::HasConjunctiveCondition E>
void NumericConstraintSelectorNode_T<E>::visit_impl(INodeVisitor<E>& visitor) const
{
    visitor.accept(*this);
}

template<formalism::HasConjunctiveCondition E>
NumericConstraintSelectorNode_T<E>::NumericConstraintSelectorNode_T(Node<E>&& true_child, GroundNumericConstraint constraint) :
    NumericConstraintSelectorNodeBase<NumericConstraintSelectorNode_T<E>, E>(constraint),
    m_true_child(std::move(true_child))
{
    assert(m_true_child);
}

template<formalism::HasConjunctiveCondition E>
void NumericConstraintSelectorNode_T<E>::generate_applicable_actions(const DenseState& state,
                                                                     const ProblemImpl& problem,
                                                                     std::vector<const INode<E>*>& ref_applicable_nodes,
                                                                     std::vector<const E*>&) const
{
    if (evaluate(this->m_constraint, problem.get_initial_function_to_value<StaticTag>(), state.get_numeric_variables()))
    {
        ref_applicable_nodes.push_back(m_true_child.get());
    }
}

template<formalism::HasConjunctiveCondition E>
const Node<E>& NumericConstraintSelectorNode_T<E>::get_true_child() const
{
    return m_true_child;
};

template class NumericConstraintSelectorNode_T<GroundActionImpl>;
template class NumericConstraintSelectorNode_T<GroundAxiomImpl>;

template<formalism::HasConjunctiveCondition E>
void NumericConstraintSelectorNode_TX<E>::visit_impl(INodeVisitor<E>& visitor) const
{
    visitor.accept(*this);
}

template<formalism::HasConjunctiveCondition E>
NumericConstraintSelectorNode_TX<E>::NumericConstraintSelectorNode_TX(Node<E>&& true_child, Node<E>&& dontcare_child, GroundNumericConstraint constraint) :
    NumericConstraintSelectorNodeBase<NumericConstraintSelectorNode_TX<E>, E>(constraint),
    m_true_child(std::move(true_child)),
    m_dontcare_child(std::move(dontcare_child))
{
    assert(m_true_child);
    assert(m_dontcare_child);
}

template<formalism::HasConjunctiveCondition E>
void NumericConstraintSelectorNode_TX<E>::generate_applicable_actions(const DenseState& state,
                                                                      const ProblemImpl& problem,
                                                                      std::vector<const INode<E>*>& ref_applicable_nodes,
                                                                      std::vector<const E*>&) const
{
    ref_applicable_nodes.push_back(m_dontcare_child.get());

    if (evaluate(this->m_constraint, problem.get_initial_function_to_value<StaticTag>(), state.get_numeric_variables()))
    {
        ref_applicable_nodes.push_back(m_true_child.get());
    }
}

template<formalism::HasConjunctiveCondition E>
const Node<E>& NumericConstraintSelectorNode_TX<E>::get_true_child() const
{
    return m_true_child;
};

template<formalism::HasConjunctiveCondition E>
const Node<E>& NumericConstraintSelectorNode_TX<E>::get_dontcare_child() const
{
    return m_dontcare_child;
}

template class NumericConstraintSelectorNode_TX<GroundActionImpl>;
template class NumericConstraintSelectorNode_TX<GroundAxiomImpl>;

}
