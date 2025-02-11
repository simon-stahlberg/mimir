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

#ifndef MIMIR_SEARCH_MATCH_TREE_CONSTRUCTION_HELPERS_NODE_CREATION_HPP_
#define MIMIR_SEARCH_MATCH_TREE_CONSTRUCTION_HELPERS_NODE_CREATION_HPP_

#include "mimir/search/match_tree/declarations.hpp"
#include "mimir/search/match_tree/nodes/atom.hpp"
#include "mimir/search/match_tree/nodes/generator.hpp"
#include "mimir/search/match_tree/nodes/numeric_constraint.hpp"

#include <deque>
#include <vector>

namespace mimir::match_tree
{

template<HasConjunctiveCondition Element>
struct StackEntry
{
    bool expanded;
    const IInverseNode<Element>* node;
};

template<HasConjunctiveCondition Element>
struct ResultTreeVisitor : public IInverseNodeVisitor<Element>
{
    std::vector<StackEntry<Element>>& m_stack;
    std::vector<Node<Element>>& m_result_stack;

    ResultTreeVisitor(std::vector<StackEntry<Element>>& stack, std::vector<Node<Element>>& result_stack) : m_stack(stack), m_result_stack(result_stack) {}

    void accept(const InverseAtomSelectorNode_TFX<Element, Fluent>& atom) override;
    void accept(const InverseAtomSelectorNode_TF<Element, Fluent>& atom) override;
    void accept(const InverseAtomSelectorNode_TX<Element, Fluent>& atom) override;
    void accept(const InverseAtomSelectorNode_FX<Element, Fluent>& atom) override;
    void accept(const InverseAtomSelectorNode_T<Element, Fluent>& atom) override;
    void accept(const InverseAtomSelectorNode_F<Element, Fluent>& atom) override;
    void accept(const InverseAtomSelectorNode_TFX<Element, Derived>& atom) override;
    void accept(const InverseAtomSelectorNode_TF<Element, Derived>& atom) override;
    void accept(const InverseAtomSelectorNode_TX<Element, Derived>& atom) override;
    void accept(const InverseAtomSelectorNode_FX<Element, Derived>& atom) override;
    void accept(const InverseAtomSelectorNode_T<Element, Derived>& atom) override;
    void accept(const InverseAtomSelectorNode_F<Element, Derived>& atom) override;
    void accept(const InverseNumericConstraintSelectorNode_T<Element>& constraint) override;
    void accept(const InverseNumericConstraintSelectorNode_TX<Element>& constraint) override;
    void accept(const InverseElementGeneratorNode<Element>& generator) override;
};

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
void accept_impl(ResultTreeVisitor<Element>& visitor, const InverseAtomSelectorNode_TFX<Element, P>& atom)
{
    auto true_child = std::move(visitor.m_result_stack.back());
    visitor.m_result_stack.pop_back();
    auto false_child = std::move(visitor.m_result_stack.back());
    visitor.m_result_stack.pop_back();
    auto dont_care_child = std::move(visitor.m_result_stack.back());
    visitor.m_result_stack.pop_back();

    // Construct new result
    visitor.m_result_stack.push_back(
        std::make_unique<AtomSelectorNode_TFX<Element, P>>(std::move(true_child), std::move(false_child), std::move(dont_care_child), atom.get_atom()));
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
void accept_impl(ResultTreeVisitor<Element>& visitor, const InverseAtomSelectorNode_TF<Element, P>& atom)
{
    auto true_child = std::move(visitor.m_result_stack.back());
    visitor.m_result_stack.pop_back();
    auto false_child = std::move(visitor.m_result_stack.back());
    visitor.m_result_stack.pop_back();
    // Construct new result
    visitor.m_result_stack.push_back(std::make_unique<AtomSelectorNode_TF<Element, P>>(std::move(true_child), std::move(false_child), atom.get_atom()));
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
void accept_impl(ResultTreeVisitor<Element>& visitor, const InverseAtomSelectorNode_TX<Element, P>& atom)
{
    auto true_child = std::move(visitor.m_result_stack.back());
    visitor.m_result_stack.pop_back();
    auto dont_care_child = std::move(visitor.m_result_stack.back());
    visitor.m_result_stack.pop_back();
    // Construct new result
    visitor.m_result_stack.push_back(std::make_unique<AtomSelectorNode_TX<Element, P>>(std::move(true_child), std::move(dont_care_child), atom.get_atom()));
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
void accept_impl(ResultTreeVisitor<Element>& visitor, const InverseAtomSelectorNode_FX<Element, P>& atom)
{
    auto false_child = std::move(visitor.m_result_stack.back());
    visitor.m_result_stack.pop_back();
    auto dont_care_child = std::move(visitor.m_result_stack.back());
    visitor.m_result_stack.pop_back();
    // Construct new result
    visitor.m_result_stack.push_back(std::make_unique<AtomSelectorNode_FX<Element, P>>(std::move(false_child), std::move(dont_care_child), atom.get_atom()));
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
void accept_impl(ResultTreeVisitor<Element>& visitor, const InverseAtomSelectorNode_T<Element, P>& atom)
{
    auto true_child = std::move(visitor.m_result_stack.back());
    visitor.m_result_stack.pop_back();
    // Construct new result
    visitor.m_result_stack.push_back(std::make_unique<AtomSelectorNode_T<Element, P>>(std::move(true_child), atom.get_atom()));
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
void accept_impl(ResultTreeVisitor<Element>& visitor, const InverseAtomSelectorNode_F<Element, P>& atom)
{
    auto false_child = std::move(visitor.m_result_stack.back());
    visitor.m_result_stack.pop_back();
    // Construct new result
    visitor.m_result_stack.push_back(std::make_unique<AtomSelectorNode_F<Element, P>>(std::move(false_child), atom.get_atom()));
}

template<HasConjunctiveCondition Element>
void ResultTreeVisitor<Element>::accept(const InverseAtomSelectorNode_TFX<Element, Fluent>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void ResultTreeVisitor<Element>::accept(const InverseAtomSelectorNode_TF<Element, Fluent>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void ResultTreeVisitor<Element>::accept(const InverseAtomSelectorNode_TX<Element, Fluent>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void ResultTreeVisitor<Element>::accept(const InverseAtomSelectorNode_FX<Element, Fluent>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void ResultTreeVisitor<Element>::accept(const InverseAtomSelectorNode_T<Element, Fluent>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void ResultTreeVisitor<Element>::accept(const InverseAtomSelectorNode_F<Element, Fluent>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void ResultTreeVisitor<Element>::accept(const InverseAtomSelectorNode_TFX<Element, Derived>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void ResultTreeVisitor<Element>::accept(const InverseAtomSelectorNode_TF<Element, Derived>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void ResultTreeVisitor<Element>::accept(const InverseAtomSelectorNode_TX<Element, Derived>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void ResultTreeVisitor<Element>::accept(const InverseAtomSelectorNode_FX<Element, Derived>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void ResultTreeVisitor<Element>::accept(const InverseAtomSelectorNode_T<Element, Derived>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void ResultTreeVisitor<Element>::accept(const InverseAtomSelectorNode_F<Element, Derived>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void ResultTreeVisitor<Element>::accept(const InverseNumericConstraintSelectorNode_TX<Element>& constraint)
{
    auto true_child = std::move(m_result_stack.back());
    m_result_stack.pop_back();
    auto dont_care_child = std::move(m_result_stack.back());
    m_result_stack.pop_back();
    // Construct new result
    m_result_stack.push_back(
        std::make_unique<NumericConstraintSelectorNode_TX<Element>>(std::move(true_child), std::move(dont_care_child), constraint.get_constraint()));
}

template<HasConjunctiveCondition Element>
void ResultTreeVisitor<Element>::accept(const InverseNumericConstraintSelectorNode_T<Element>& constraint)
{
    auto true_child = std::move(m_result_stack.back());
    m_result_stack.pop_back();
    // Construct new result
    m_result_stack.push_back(std::make_unique<NumericConstraintSelectorNode_T<Element>>(std::move(true_child), constraint.get_constraint()));
}

template<HasConjunctiveCondition Element>
void ResultTreeVisitor<Element>::accept(const InverseElementGeneratorNode<Element>& generator)
{
    assert(!generator.get_elements().empty());
    m_result_stack.push_back(std::make_unique<ElementGeneratorNode<Element>>(generator.get_elements()));
}

template<HasConjunctiveCondition Element>
struct ExpandTreeVisitor : public IInverseNodeVisitor<Element>
{
    std::vector<StackEntry<Element>>& m_stack;

    ExpandTreeVisitor(std::vector<StackEntry<Element>>& stack) : m_stack(stack) {}

    void accept(const InverseAtomSelectorNode_TFX<Element, Fluent>& atom) override;
    void accept(const InverseAtomSelectorNode_TF<Element, Fluent>& atom) override;
    void accept(const InverseAtomSelectorNode_TX<Element, Fluent>& atom) override;
    void accept(const InverseAtomSelectorNode_FX<Element, Fluent>& atom) override;
    void accept(const InverseAtomSelectorNode_T<Element, Fluent>& atom) override;
    void accept(const InverseAtomSelectorNode_F<Element, Fluent>& atom) override;
    void accept(const InverseAtomSelectorNode_TFX<Element, Derived>& atom) override;
    void accept(const InverseAtomSelectorNode_TF<Element, Derived>& atom) override;
    void accept(const InverseAtomSelectorNode_TX<Element, Derived>& atom) override;
    void accept(const InverseAtomSelectorNode_FX<Element, Derived>& atom) override;
    void accept(const InverseAtomSelectorNode_T<Element, Derived>& atom) override;
    void accept(const InverseAtomSelectorNode_F<Element, Derived>& atom) override;
    void accept(const InverseNumericConstraintSelectorNode_T<Element>& constraint) override;
    void accept(const InverseNumericConstraintSelectorNode_TX<Element>& constraint) override;
    void accept(const InverseElementGeneratorNode<Element>& generator) override;
};

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
void accept_impl(ExpandTreeVisitor<Element>& visitor, const InverseAtomSelectorNode_TFX<Element, P>& atom)
{
    visitor.m_stack.push_back(StackEntry<Element> { false, atom.get_true_child().get() });
    visitor.m_stack.push_back(StackEntry<Element> { false, atom.get_false_child().get() });
    visitor.m_stack.push_back(StackEntry<Element> { false, atom.get_dontcare_child().get() });
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
void accept_impl(ExpandTreeVisitor<Element>& visitor, const InverseAtomSelectorNode_TF<Element, P>& atom)
{
    visitor.m_stack.push_back(StackEntry<Element> { false, atom.get_true_child().get() });
    visitor.m_stack.push_back(StackEntry<Element> { false, atom.get_false_child().get() });
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
void accept_impl(ExpandTreeVisitor<Element>& visitor, const InverseAtomSelectorNode_TX<Element, P>& atom)
{
    visitor.m_stack.push_back(StackEntry<Element> { false, atom.get_true_child().get() });
    visitor.m_stack.push_back(StackEntry<Element> { false, atom.get_dontcare_child().get() });
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
void accept_impl(ExpandTreeVisitor<Element>& visitor, const InverseAtomSelectorNode_FX<Element, P>& atom)
{
    visitor.m_stack.push_back(StackEntry<Element> { false, atom.get_false_child().get() });
    visitor.m_stack.push_back(StackEntry<Element> { false, atom.get_dontcare_child().get() });
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
void accept_impl(ExpandTreeVisitor<Element>& visitor, const InverseAtomSelectorNode_T<Element, P>& atom)
{
    visitor.m_stack.push_back(StackEntry<Element> { false, atom.get_true_child().get() });
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
void accept_impl(ExpandTreeVisitor<Element>& visitor, const InverseAtomSelectorNode_F<Element, P>& atom)
{
    visitor.m_stack.push_back(StackEntry<Element> { false, atom.get_false_child().get() });
}

template<HasConjunctiveCondition Element>
void ExpandTreeVisitor<Element>::accept(const InverseAtomSelectorNode_TFX<Element, Fluent>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void ExpandTreeVisitor<Element>::accept(const InverseAtomSelectorNode_TF<Element, Fluent>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void ExpandTreeVisitor<Element>::accept(const InverseAtomSelectorNode_TX<Element, Fluent>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void ExpandTreeVisitor<Element>::accept(const InverseAtomSelectorNode_FX<Element, Fluent>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void ExpandTreeVisitor<Element>::accept(const InverseAtomSelectorNode_T<Element, Fluent>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void ExpandTreeVisitor<Element>::accept(const InverseAtomSelectorNode_F<Element, Fluent>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void ExpandTreeVisitor<Element>::accept(const InverseAtomSelectorNode_TFX<Element, Derived>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void ExpandTreeVisitor<Element>::accept(const InverseAtomSelectorNode_TF<Element, Derived>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void ExpandTreeVisitor<Element>::accept(const InverseAtomSelectorNode_TX<Element, Derived>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void ExpandTreeVisitor<Element>::accept(const InverseAtomSelectorNode_FX<Element, Derived>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void ExpandTreeVisitor<Element>::accept(const InverseAtomSelectorNode_T<Element, Derived>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void ExpandTreeVisitor<Element>::accept(const InverseAtomSelectorNode_F<Element, Derived>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void ExpandTreeVisitor<Element>::accept(const InverseNumericConstraintSelectorNode_TX<Element>& constraint)
{
    m_stack.push_back(StackEntry<Element> { false, constraint.get_true_child().get() });
    m_stack.push_back(StackEntry<Element> { false, constraint.get_dontcare_child().get() });
}

template<HasConjunctiveCondition Element>
void ExpandTreeVisitor<Element>::accept(const InverseNumericConstraintSelectorNode_T<Element>& constraint)
{
    m_stack.push_back(StackEntry<Element> { false, constraint.get_true_child().get() });
}

template<HasConjunctiveCondition Element>
void ExpandTreeVisitor<Element>::accept(const InverseElementGeneratorNode<Element>& generator)
{
    // Nothing to be done
}

template<HasConjunctiveCondition Element>
Node<Element> parse_inverse_tree_iteratively(const InverseNode<Element>& root)
{
    auto stack = std::vector<StackEntry<Element>> { StackEntry<Element> { false, root.get() } };
    auto result_stack = std::vector<Node<Element>> {};

    while (!stack.empty())
    {
        auto& entry = stack.back();
        const auto cur_node = entry.node;  // must copy because entry might be invalidated during visitation.

        if (!entry.expanded)
        {
            // Expand the node
            entry.expanded = true;  // must set this first because entry might be invalidated during visitation.

            auto visitor = ExpandTreeVisitor<Element>(stack);
            cur_node->visit(visitor);
        }
        else
        {
            // compute the result into the result_stack.

            stack.pop_back();  ///< We can already pop here to avoid duplication in the visitor.

            auto visitor = ResultTreeVisitor<Element>(stack, result_stack);
            cur_node->visit(visitor);
        }
    }
    // Only the root node should be in the result stack.
    assert(result_stack.size() == 1);

    // std::cout << std::make_tuple(std::cref(result_stack.back()), DotPrinterTag {}) << std::endl;

    return std::move(result_stack.back());
}

template<HasConjunctiveCondition Element>
Node<Element> create_root_generator_node(std::span<const Element*> elements)
{
    return std::make_unique<ElementGeneratorNode<Element>>(elements);
}
}

#endif
