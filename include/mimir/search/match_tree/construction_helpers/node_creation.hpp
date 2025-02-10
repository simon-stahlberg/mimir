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
private:
    std::vector<StackEntry<Element>>& m_stack;
    std::vector<Node<Element>>& m_result_stack;

public:
    ResultTreeVisitor(std::vector<StackEntry<Element>>& stack, std::vector<Node<Element>>& result_stack) : m_stack(stack), m_result_stack(result_stack) {}

    void accept(const InverseAtomSelectorNode<Element, Fluent>& atom) override
    {  // Pop back in inverse order of insertion.
        auto dont_care_child = std::move(m_result_stack.back());
        m_result_stack.pop_back();
        auto false_child = std::move(m_result_stack.back());
        m_result_stack.pop_back();
        auto true_child = std::move(m_result_stack.back());
        m_result_stack.pop_back();
        // Construct new result
        m_result_stack.push_back(
            std::make_unique<AtomSelectorNode<Element, Fluent>>(std::move(true_child), std::move(false_child), std::move(dont_care_child), atom.get_atom()));
    }
    void accept(const InverseAtomSelectorNode<Element, Derived>& atom) override
    {  // Pop back in inverse order of insertion.
        auto dont_care_child = std::move(m_result_stack.back());
        m_result_stack.pop_back();
        auto false_child = std::move(m_result_stack.back());
        m_result_stack.pop_back();
        auto true_child = std::move(m_result_stack.back());
        m_result_stack.pop_back();
        // Construct new result
        m_result_stack.push_back(
            std::make_unique<AtomSelectorNode<Element, Derived>>(std::move(true_child), std::move(false_child), std::move(dont_care_child), atom.get_atom()));
    }
    void accept(const InverseNumericConstraintSelectorNode<Element>& constraint) override
    {
        // Pop back in inverse order of insertion.
        auto dont_care_child = std::move(m_result_stack.back());
        m_result_stack.pop_back();
        auto true_child = std::move(m_result_stack.back());
        m_result_stack.pop_back();
        // Construct new result
        m_result_stack.push_back(
            std::make_unique<NumericConstraintSelectorNode<Element>>(std::move(true_child), std::move(dont_care_child), constraint.get_constraint()));
    }
    void accept(const InverseElementGeneratorNode<Element>& generator) override
    {
        if (generator.get_elements().empty())
        {
            m_result_stack.push_back(nullptr);  ///< skip creating an empty generator node.
        }
        else
        {
            m_result_stack.push_back(std::make_unique<ElementGeneratorNode<Element>>(generator.get_elements()));
        }
    }
};

template<HasConjunctiveCondition Element>
struct ExpandTreeVisitor : public IInverseNodeVisitor<Element>
{
    std::vector<StackEntry<Element>>& m_stack;

    ExpandTreeVisitor(std::vector<StackEntry<Element>>& stack) : m_stack(stack) {}

    void accept(const InverseAtomSelectorNode<Element, Fluent>& atom) override
    {
        m_stack.push_back(StackEntry<Element> { false, atom.get_true_child().get() });
        m_stack.push_back(StackEntry<Element> { false, atom.get_false_child().get() });
        m_stack.push_back(StackEntry<Element> { false, atom.get_dontcare_child().get() });
    }
    void accept(const InverseAtomSelectorNode<Element, Derived>& atom) override
    {
        m_stack.push_back(StackEntry<Element> { false, atom.get_true_child().get() });
        m_stack.push_back(StackEntry<Element> { false, atom.get_false_child().get() });
        m_stack.push_back(StackEntry<Element> { false, atom.get_dontcare_child().get() });
    }
    void accept(const InverseNumericConstraintSelectorNode<Element>& constraint) override
    {
        m_stack.push_back(StackEntry<Element> { false, constraint.get_true_child().get() });
        m_stack.push_back(StackEntry<Element> { false, constraint.get_dontcare_child().get() });
    }
    void accept(const InverseElementGeneratorNode<Element>& generator) override {}
};

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

            if (cur_node)  ///< only expand existing nodes, i.e., not nullptr
            {
                auto visitor = ExpandTreeVisitor<Element>(stack);
                cur_node->visit(visitor);
            }
            else
            {
                stack.pop_back();
                result_stack.push_back(nullptr);
            }
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

    return std::move(result_stack.back());
}

template<HasConjunctiveCondition Element>
Node<Element> create_root_generator_node(std::span<const Element*> elements)
{
    return std::make_unique<ElementGeneratorNode<Element>>(elements);
}
}

#endif
