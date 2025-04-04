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

#include "mimir/formalism/declarations.hpp"
#include "mimir/search/match_tree/nodes/interface.hpp"

namespace mimir::search::match_tree
{
template<typename Derived_, formalism::HasConjunctiveCondition E>
class NumericConstraintSelectorNodeBase : public INode<E>
{
private:
    /// @brief Helper to cast to Derived_.
    constexpr const auto& self() const { return static_cast<const Derived_&>(*this); }
    constexpr auto& self() { return static_cast<Derived_&>(*this); }

protected:
    formalism::GroundNumericConstraint m_constraint;

public:
    explicit NumericConstraintSelectorNodeBase(formalism::GroundNumericConstraint constraint) : m_constraint(constraint) { assert(m_constraint); }

    NumericConstraintSelectorNodeBase(const NumericConstraintSelectorNodeBase& other) = delete;
    NumericConstraintSelectorNodeBase& operator=(const NumericConstraintSelectorNodeBase& other) = delete;
    NumericConstraintSelectorNodeBase(NumericConstraintSelectorNodeBase&& other) = delete;
    NumericConstraintSelectorNodeBase& operator=(NumericConstraintSelectorNodeBase&& other) = delete;

    formalism::GroundNumericConstraint get_constraint() const { return m_constraint; }

    void visit(INodeVisitor<E>& visitor) const override { self().visit_impl(visitor); }
};

template<formalism::HasConjunctiveCondition E>
class NumericConstraintSelectorNode_T : public NumericConstraintSelectorNodeBase<NumericConstraintSelectorNode_T<E>, E>
{
private:
    Node<E> m_true_child;

private:
    /* Implement interface*/

    void visit_impl(INodeVisitor<E>& visitor) const;

    friend class NumericConstraintSelectorNodeBase<NumericConstraintSelectorNode_T<E>, E>;

public:
    using NumericConstraintSelectorNodeBase<NumericConstraintSelectorNode_T<E>, E>::get_constraint;

    NumericConstraintSelectorNode_T(Node<E>&& true_child, formalism::GroundNumericConstraint constraint);

    void generate_applicable_actions(const DenseState& state,
                                     const formalism::ProblemImpl& problem,
                                     std::vector<const INode<E>*>& ref_applicable_nodes,
                                     std::vector<const E*>&) const override;

    const Node<E>& get_true_child() const;
};

template<formalism::HasConjunctiveCondition E>
class NumericConstraintSelectorNode_TX : public NumericConstraintSelectorNodeBase<NumericConstraintSelectorNode_TX<E>, E>
{
private:
    Node<E> m_true_child;
    Node<E> m_dontcare_child;

private:
    /* Implement interface*/

    void visit_impl(INodeVisitor<E>& visitor) const;

    friend class NumericConstraintSelectorNodeBase<NumericConstraintSelectorNode_TX<E>, E>;

public:
    using NumericConstraintSelectorNodeBase<NumericConstraintSelectorNode_TX<E>, E>::get_constraint;

    NumericConstraintSelectorNode_TX(Node<E>&& true_child, Node<E>&& dontcare_child, formalism::GroundNumericConstraint constraint);

    void generate_applicable_actions(const DenseState& state,
                                     const formalism::ProblemImpl& problem,
                                     std::vector<const INode<E>*>& ref_applicable_nodes,
                                     std::vector<const E*>&) const override;

    const Node<E>& get_true_child() const;
    const Node<E>& get_dontcare_child() const;
};
}

#endif
