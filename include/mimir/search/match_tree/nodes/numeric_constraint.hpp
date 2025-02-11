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
template<typename Derived_, HasConjunctiveCondition Element>
class NumericConstraintSelectorNodeBase : public INode<Element>
{
private:
    /// @brief Helper to cast to Derived_.
    constexpr const auto& self() const { return static_cast<const Derived_&>(*this); }
    constexpr auto& self() { return static_cast<Derived_&>(*this); }

protected:
    GroundNumericConstraint m_constraint;

public:
    explicit NumericConstraintSelectorNodeBase(GroundNumericConstraint constraint) : m_constraint(constraint) { assert(m_constraint); }

    GroundNumericConstraint get_constraint() const { return m_constraint; }

    void visit(INodeVisitor<Element>& visitor) const override { self().visit_impl(visitor); }
};

template<HasConjunctiveCondition Element>
class NumericConstraintSelectorNode_T : public NumericConstraintSelectorNodeBase<NumericConstraintSelectorNode_T<Element>, Element>
{
private:
    Node<Element> m_true_child;

private:
    /* Implement interface*/

    void visit_impl(INodeVisitor<Element>& visitor) const { visitor.accept(*this); }

    friend class NumericConstraintSelectorNodeBase<NumericConstraintSelectorNode_T<Element>, Element>;

public:
    using NumericConstraintSelectorNodeBase<NumericConstraintSelectorNode_T<Element>, Element>::get_constraint;

    NumericConstraintSelectorNode_T(Node<Element>&& true_child, GroundNumericConstraint constraint) :
        NumericConstraintSelectorNodeBase<NumericConstraintSelectorNode_T<Element>, Element>(constraint),
        m_true_child(std::move(true_child))
    {
        assert(m_true_child);
    }

    void
    generate_applicable_actions(const DenseState& state, std::vector<const INode<Element>*>& ref_applicable_nodes, std::vector<const Element*>&) const override
    {
        if (evaluate(this->m_constraint, state.get_numeric_variables()))
        {
            ref_applicable_nodes.push_back(m_true_child.get());
        }
    }

    const Node<Element>& get_true_child() const { return m_true_child; };
};

template<HasConjunctiveCondition Element>
class NumericConstraintSelectorNode_TX : public NumericConstraintSelectorNodeBase<NumericConstraintSelectorNode_TX<Element>, Element>
{
private:
    Node<Element> m_true_child;
    Node<Element> m_dontcare_child;

private:
    /* Implement interface*/

    void visit_impl(INodeVisitor<Element>& visitor) const { visitor.accept(*this); }

    friend class NumericConstraintSelectorNodeBase<NumericConstraintSelectorNode_TX<Element>, Element>;

public:
    using NumericConstraintSelectorNodeBase<NumericConstraintSelectorNode_TX<Element>, Element>::get_constraint;

    NumericConstraintSelectorNode_TX(Node<Element>&& true_child, Node<Element>&& dontcare_child, GroundNumericConstraint constraint) :
        NumericConstraintSelectorNodeBase<NumericConstraintSelectorNode_TX<Element>, Element>(constraint),
        m_true_child(std::move(true_child)),
        m_dontcare_child(std::move(dontcare_child))
    {
        assert(m_true_child);
        assert(m_dontcare_child);
    }

    void
    generate_applicable_actions(const DenseState& state, std::vector<const INode<Element>*>& ref_applicable_nodes, std::vector<const Element*>&) const override
    {
        ref_applicable_nodes.push_back(m_dontcare_child.get());

        if (evaluate(this->m_constraint, state.get_numeric_variables()))
        {
            ref_applicable_nodes.push_back(m_true_child.get());
        }
    }

    const Node<Element>& get_true_child() const { return m_true_child; };
    const Node<Element>& get_dontcare_child() const { return m_dontcare_child; }
};
}

#endif
