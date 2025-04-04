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

#include "mimir/search/match_tree/construction_helpers/inverse_nodes/numeric_constraint.hpp"

#include "mimir/formalism/ground_action.hpp"
#include "mimir/formalism/ground_axiom.hpp"
#include "mimir/formalism/ground_numeric_constraint.hpp"

using namespace mimir::formalism;

namespace mimir::search::match_tree
{

template<formalism::HasConjunctiveCondition E>
void InverseNumericConstraintSelectorNode_T<E>::visit_impl(IInverseNodeVisitor<E>& visitor) const
{
    visitor.accept(*this);
}

template<formalism::HasConjunctiveCondition E>
InverseNumericConstraintSelectorNode_T<E>::InverseNumericConstraintSelectorNode_T(const IInverseNode<E>* parent,
                                                                                  SplitList useless_splits,
                                                                                  GroundNumericConstraint constraint,
                                                                                  std::span<const E*> true_elements) :
    InverseNumericConstraintSelectorNodeBase<InverseNumericConstraintSelectorNode_T<E>, E>(parent, std::move(useless_splits), constraint),
    m_true_elements(true_elements),
    m_true_child(nullptr)
{
    assert(!m_true_elements.empty());
}

template<formalism::HasConjunctiveCondition E>
std::span<const E*> InverseNumericConstraintSelectorNode_T<E>::get_true_elements() const
{
    return m_true_elements;
}

template<formalism::HasConjunctiveCondition E>
InverseNode<E>& InverseNumericConstraintSelectorNode_T<E>::get_true_child()
{
    return m_true_child;
};

template<formalism::HasConjunctiveCondition E>
const InverseNode<E>& InverseNumericConstraintSelectorNode_T<E>::get_true_child() const
{
    return m_true_child;
};

template class InverseNumericConstraintSelectorNode_T<GroundActionImpl>;
template class InverseNumericConstraintSelectorNode_T<GroundAxiomImpl>;

template<formalism::HasConjunctiveCondition E>
void InverseNumericConstraintSelectorNode_TX<E>::visit_impl(IInverseNodeVisitor<E>& visitor) const
{
    visitor.accept(*this);
}

template<formalism::HasConjunctiveCondition E>
InverseNumericConstraintSelectorNode_TX<E>::InverseNumericConstraintSelectorNode_TX(const IInverseNode<E>* parent,
                                                                                    SplitList useless_splits,
                                                                                    GroundNumericConstraint constraint,
                                                                                    std::span<const E*> true_elements,
                                                                                    std::span<const E*> dontcare_elements) :
    InverseNumericConstraintSelectorNodeBase<InverseNumericConstraintSelectorNode_TX<E>, E>(parent, std::move(useless_splits), constraint),
    m_true_elements(true_elements),
    m_dontcare_elements(dontcare_elements),
    m_true_child(nullptr),
    m_dontcare_child(nullptr)
{
    assert(!m_true_elements.empty());
    assert(!m_dontcare_elements.empty());
}

template<formalism::HasConjunctiveCondition E>
std::span<const E*> InverseNumericConstraintSelectorNode_TX<E>::get_true_elements() const
{
    return m_true_elements;
}

template<formalism::HasConjunctiveCondition E>
std::span<const E*> InverseNumericConstraintSelectorNode_TX<E>::get_dontcare_elements() const
{
    return m_dontcare_elements;
}

template<formalism::HasConjunctiveCondition E>
InverseNode<E>& InverseNumericConstraintSelectorNode_TX<E>::get_true_child()
{
    return m_true_child;
};

template<formalism::HasConjunctiveCondition E>
InverseNode<E>& InverseNumericConstraintSelectorNode_TX<E>::get_dontcare_child()
{
    return m_dontcare_child;
}

template<formalism::HasConjunctiveCondition E>
const InverseNode<E>& InverseNumericConstraintSelectorNode_TX<E>::get_true_child() const
{
    return m_true_child;
};

template<formalism::HasConjunctiveCondition E>
const InverseNode<E>& InverseNumericConstraintSelectorNode_TX<E>::get_dontcare_child() const
{
    return m_dontcare_child;
}

template class InverseNumericConstraintSelectorNode_TX<GroundActionImpl>;
template class InverseNumericConstraintSelectorNode_TX<GroundAxiomImpl>;

}
