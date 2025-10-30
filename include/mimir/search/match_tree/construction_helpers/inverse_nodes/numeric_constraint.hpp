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

#ifndef MIMIR_SEARCH_MATCH_TREE_CONSTRUCTION_HELPERS_INVERSE_NODES_NUMERIC_CONSTRAINT_HPP_
#define MIMIR_SEARCH_MATCH_TREE_CONSTRUCTION_HELPERS_INVERSE_NODES_NUMERIC_CONSTRAINT_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/search/match_tree/construction_helpers/inverse_nodes/interface.hpp"

namespace mimir::search::match_tree
{
template<typename Derived_, formalism::HasConjunctiveCondition E>
class InverseNumericConstraintSelectorNodeBase : public IInverseNode<E>
{
private:
    /// @brief Helper to cast to Derived_.
    constexpr const auto& self() const { return static_cast<const Derived_&>(*this); }
    constexpr auto& self() { return static_cast<Derived_&>(*this); }

protected:
    // Meta data
    formalism::GroundNumericConstraint m_constraint;

public:
    InverseNumericConstraintSelectorNodeBase(const IInverseNode<E>* parent, SplitList useless_splits, formalism::GroundNumericConstraint constraint) :
        IInverseNode<E>(parent, std::move(useless_splits)),
        m_constraint(constraint)
    {
        assert(m_constraint);
    }
    InverseNumericConstraintSelectorNodeBase(const InverseNumericConstraintSelectorNodeBase& other) = delete;
    InverseNumericConstraintSelectorNodeBase& operator=(const InverseNumericConstraintSelectorNodeBase& other) = delete;
    InverseNumericConstraintSelectorNodeBase(InverseNumericConstraintSelectorNodeBase&& other) = delete;
    InverseNumericConstraintSelectorNodeBase& operator=(InverseNumericConstraintSelectorNodeBase&& other) = delete;

    void visit(IInverseNodeVisitor<E>& visitor) const override { self().visit_impl(visitor); }

    formalism::GroundNumericConstraint get_constraint() const { return m_constraint; }
};

template<formalism::HasConjunctiveCondition E>
class InverseNumericConstraintSelectorNode_T : public InverseNumericConstraintSelectorNodeBase<InverseNumericConstraintSelectorNode_T<E>, E>
{
private:
    // Candidates for further refinement.
    std::span<const E*> m_true_elements;

    InverseNode<E> m_true_child;

private:
    /* Implement interface*/
    void visit_impl(IInverseNodeVisitor<E>& visitor) const;

    friend class InverseNumericConstraintSelectorNodeBase<InverseNumericConstraintSelectorNode_T<E>, E>;

public:
    using InverseNumericConstraintSelectorNodeBase<InverseNumericConstraintSelectorNode_T<E>, E>::get_constraint;

    InverseNumericConstraintSelectorNode_T(const IInverseNode<E>* parent,
                                           SplitList useless_splits,
                                           formalism::GroundNumericConstraint constraint,
                                           std::span<const E*> true_elements);

    std::span<const E*> get_true_elements() const;

    InverseNode<E>& get_true_child();

    const InverseNode<E>& get_true_child() const;
};

template<formalism::HasConjunctiveCondition E>
class InverseNumericConstraintSelectorNode_TX : public InverseNumericConstraintSelectorNodeBase<InverseNumericConstraintSelectorNode_TX<E>, E>
{
private:
    // Candidates for further refinement.
    std::span<const E*> m_true_elements;
    std::span<const E*> m_dontcare_elements;

    InverseNode<E> m_true_child;
    InverseNode<E> m_dontcare_child;

private:
    /* Implement interface*/
    void visit_impl(IInverseNodeVisitor<E>& visitor) const;

    friend class InverseNumericConstraintSelectorNodeBase<InverseNumericConstraintSelectorNode_TX<E>, E>;

public:
    using InverseNumericConstraintSelectorNodeBase<InverseNumericConstraintSelectorNode_TX<E>, E>::get_constraint;

    InverseNumericConstraintSelectorNode_TX(const IInverseNode<E>* parent,
                                            SplitList useless_splits,
                                            formalism::GroundNumericConstraint constraint,
                                            std::span<const E*> true_elements,
                                            std::span<const E*> dontcare_elements);

    std::span<const E*> get_true_elements() const;
    std::span<const E*> get_dontcare_elements() const;

    InverseNode<E>& get_true_child();
    InverseNode<E>& get_dontcare_child();

    const InverseNode<E>& get_true_child() const;
    const InverseNode<E>& get_dontcare_child() const;
};
}

#endif
