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

#include "mimir/formalism/ground_numeric_constraint.hpp"
#include "mimir/search/match_tree/construction_helpers/inverse_nodes/interface.hpp"

namespace mimir::match_tree
{
template<typename Derived_, HasConjunctiveCondition Element>
class InverseNumericConstraintSelectorNodeBase : public IInverseNode<Element>
{
private:
    /// @brief Helper to cast to Derived_.
    constexpr const auto& self() const { return static_cast<const Derived_&>(*this); }
    constexpr auto& self() { return static_cast<Derived_&>(*this); }

protected:
    // Meta data
    GroundNumericConstraint m_constraint;

public:
    InverseNumericConstraintSelectorNodeBase(const IInverseNode<Element>* parent,
                                             SplitList useless_splits,
                                             size_t root_distance,
                                             GroundNumericConstraint constraint) :
        IInverseNode<Element>(parent, std::move(useless_splits), root_distance),
        m_constraint(constraint)
    {
        assert(m_constraint);
    }
    InverseNumericConstraintSelectorNodeBase(const InverseNumericConstraintSelectorNodeBase& other) = delete;
    InverseNumericConstraintSelectorNodeBase& operator=(const InverseNumericConstraintSelectorNodeBase& other) = delete;
    InverseNumericConstraintSelectorNodeBase(InverseNumericConstraintSelectorNodeBase&& other) = delete;
    InverseNumericConstraintSelectorNodeBase& operator=(InverseNumericConstraintSelectorNodeBase&& other) = delete;

    void visit(IInverseNodeVisitor<Element>& visitor) const override { self().visit_impl(visitor); }

    GroundNumericConstraint get_constraint() const { return m_constraint; }
};

template<HasConjunctiveCondition Element>
class InverseNumericConstraintSelectorNode_T : public InverseNumericConstraintSelectorNodeBase<InverseNumericConstraintSelectorNode_T<Element>, Element>
{
private:
    // Candidates for further refinement.
    std::span<const Element*> m_true_elements;

    InverseNode<Element> m_true_child;

private:
    /* Implement interface*/
    void visit_impl(IInverseNodeVisitor<Element>& visitor) const { visitor.accept(*this); }

    friend class InverseNumericConstraintSelectorNodeBase<InverseNumericConstraintSelectorNode_T<Element>, Element>;

public:
    using InverseNumericConstraintSelectorNodeBase<InverseNumericConstraintSelectorNode_T<Element>, Element>::get_constraint;

    InverseNumericConstraintSelectorNode_T(const IInverseNode<Element>* parent,
                                           SplitList useless_splits,
                                           size_t root_distance,
                                           GroundNumericConstraint constraint,
                                           std::span<const Element*> true_elements) :
        InverseNumericConstraintSelectorNodeBase<InverseNumericConstraintSelectorNode_T<Element>, Element>(parent,
                                                                                                           std::move(useless_splits),
                                                                                                           root_distance,
                                                                                                           constraint),
        m_true_elements(true_elements),
        m_true_child(nullptr)
    {
        assert(!m_true_elements.empty());
    }

    std::span<const Element*> get_true_elements() const { return m_true_elements; }

    InverseNode<Element>& get_true_child() { return m_true_child; };

    const InverseNode<Element>& get_true_child() const { return m_true_child; };
};

template<HasConjunctiveCondition Element>
class InverseNumericConstraintSelectorNode_TX : public InverseNumericConstraintSelectorNodeBase<InverseNumericConstraintSelectorNode_TX<Element>, Element>
{
private:
    // Candidates for further refinement.
    std::span<const Element*> m_true_elements;
    std::span<const Element*> m_dontcare_elements;

    InverseNode<Element> m_true_child;
    InverseNode<Element> m_dontcare_child;

private:
    /* Implement interface*/
    void visit_impl(IInverseNodeVisitor<Element>& visitor) const { visitor.accept(*this); }

    friend class InverseNumericConstraintSelectorNodeBase<InverseNumericConstraintSelectorNode_TX<Element>, Element>;

public:
    using InverseNumericConstraintSelectorNodeBase<InverseNumericConstraintSelectorNode_TX<Element>, Element>::get_constraint;

    InverseNumericConstraintSelectorNode_TX(const IInverseNode<Element>* parent,
                                            SplitList useless_splits,
                                            size_t root_distance,
                                            GroundNumericConstraint constraint,
                                            std::span<const Element*> true_elements,
                                            std::span<const Element*> dontcare_elements) :
        InverseNumericConstraintSelectorNodeBase<InverseNumericConstraintSelectorNode_TX<Element>, Element>(parent,
                                                                                                            std::move(useless_splits),
                                                                                                            root_distance,
                                                                                                            constraint),
        m_true_elements(true_elements),
        m_dontcare_elements(dontcare_elements),
        m_true_child(nullptr),
        m_dontcare_child(nullptr)
    {
        assert(!m_true_elements.empty());
        assert(!m_dontcare_elements.empty());
    }

    std::span<const Element*> get_true_elements() const { return m_true_elements; }
    std::span<const Element*> get_dontcare_elements() const { return m_dontcare_elements; }

    InverseNode<Element>& get_true_child() { return m_true_child; };
    InverseNode<Element>& get_dontcare_child() { return m_dontcare_child; }

    const InverseNode<Element>& get_true_child() const { return m_true_child; };
    const InverseNode<Element>& get_dontcare_child() const { return m_dontcare_child; }
};
}

#endif
