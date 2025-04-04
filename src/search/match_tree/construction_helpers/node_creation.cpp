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

#include "mimir/search/match_tree/construction_helpers/node_creation.hpp"

#include "mimir/formalism/ground_action.hpp"
#include "mimir/formalism/ground_axiom.hpp"
#include "mimir/search/match_tree/construction_helpers/inverse_nodes/atom.hpp"
#include "mimir/search/match_tree/construction_helpers/inverse_nodes/generator.hpp"
#include "mimir/search/match_tree/construction_helpers/inverse_nodes/numeric_constraint.hpp"
#include "mimir/search/match_tree/declarations.hpp"
#include "mimir/search/match_tree/nodes/atom.hpp"
#include "mimir/search/match_tree/nodes/generator.hpp"
#include "mimir/search/match_tree/nodes/numeric_constraint.hpp"

#include <deque>
#include <vector>

using namespace mimir::formalism;

namespace mimir::search::match_tree
{

template<formalism::HasConjunctiveCondition E>
struct NodeCreationStackEntry
{
    bool expanded;
    const IInverseNode<E>* node;
};

template<formalism::HasConjunctiveCondition E>
struct ResultInverseTreeVisitor : public IInverseNodeVisitor<E>
{
    std::vector<NodeCreationStackEntry<E>>& m_stack;
    std::vector<Node<E>>& m_result_stack;

    ResultInverseTreeVisitor(std::vector<NodeCreationStackEntry<E>>& stack, std::vector<Node<E>>& result_stack) : m_stack(stack), m_result_stack(result_stack)
    {
    }

    void accept(const InverseAtomSelectorNode_TFX<E, FluentTag>& atom) override;
    void accept(const InverseAtomSelectorNode_TF<E, FluentTag>& atom) override;
    void accept(const InverseAtomSelectorNode_TX<E, FluentTag>& atom) override;
    void accept(const InverseAtomSelectorNode_FX<E, FluentTag>& atom) override;
    void accept(const InverseAtomSelectorNode_T<E, FluentTag>& atom) override;
    void accept(const InverseAtomSelectorNode_F<E, FluentTag>& atom) override;
    void accept(const InverseAtomSelectorNode_TFX<E, DerivedTag>& atom) override;
    void accept(const InverseAtomSelectorNode_TF<E, DerivedTag>& atom) override;
    void accept(const InverseAtomSelectorNode_TX<E, DerivedTag>& atom) override;
    void accept(const InverseAtomSelectorNode_FX<E, DerivedTag>& atom) override;
    void accept(const InverseAtomSelectorNode_T<E, DerivedTag>& atom) override;
    void accept(const InverseAtomSelectorNode_F<E, DerivedTag>& atom) override;
    void accept(const InverseNumericConstraintSelectorNode_T<E>& constraint) override;
    void accept(const InverseNumericConstraintSelectorNode_TX<E>& constraint) override;
    void accept(const InverseElementGeneratorNode_Perfect<E>& generator) override;
    void accept(const InverseElementGeneratorNode_Imperfect<E>& generator) override;
};

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
void accept_impl(ResultInverseTreeVisitor<E>& visitor, const InverseAtomSelectorNode_TFX<E, P>& atom)
{
    auto true_child = std::move(visitor.m_result_stack.back());
    visitor.m_result_stack.pop_back();
    auto false_child = std::move(visitor.m_result_stack.back());
    visitor.m_result_stack.pop_back();
    auto dont_care_child = std::move(visitor.m_result_stack.back());
    visitor.m_result_stack.pop_back();

    // Construct new result
    visitor.m_result_stack.push_back(
        std::make_unique<AtomSelectorNode_TFX<E, P>>(std::move(true_child), std::move(false_child), std::move(dont_care_child), atom.get_atom()));
}

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
void accept_impl(ResultInverseTreeVisitor<E>& visitor, const InverseAtomSelectorNode_TF<E, P>& atom)
{
    auto true_child = std::move(visitor.m_result_stack.back());
    visitor.m_result_stack.pop_back();
    auto false_child = std::move(visitor.m_result_stack.back());
    visitor.m_result_stack.pop_back();
    // Construct new result
    visitor.m_result_stack.push_back(std::make_unique<AtomSelectorNode_TF<E, P>>(std::move(true_child), std::move(false_child), atom.get_atom()));
}

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
void accept_impl(ResultInverseTreeVisitor<E>& visitor, const InverseAtomSelectorNode_TX<E, P>& atom)
{
    auto true_child = std::move(visitor.m_result_stack.back());
    visitor.m_result_stack.pop_back();
    auto dont_care_child = std::move(visitor.m_result_stack.back());
    visitor.m_result_stack.pop_back();
    // Construct new result
    visitor.m_result_stack.push_back(std::make_unique<AtomSelectorNode_TX<E, P>>(std::move(true_child), std::move(dont_care_child), atom.get_atom()));
}

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
void accept_impl(ResultInverseTreeVisitor<E>& visitor, const InverseAtomSelectorNode_FX<E, P>& atom)
{
    auto false_child = std::move(visitor.m_result_stack.back());
    visitor.m_result_stack.pop_back();
    auto dont_care_child = std::move(visitor.m_result_stack.back());
    visitor.m_result_stack.pop_back();
    // Construct new result
    visitor.m_result_stack.push_back(std::make_unique<AtomSelectorNode_FX<E, P>>(std::move(false_child), std::move(dont_care_child), atom.get_atom()));
}

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
void accept_impl(ResultInverseTreeVisitor<E>& visitor, const InverseAtomSelectorNode_T<E, P>& atom)
{
    auto true_child = std::move(visitor.m_result_stack.back());
    visitor.m_result_stack.pop_back();
    // Construct new result
    visitor.m_result_stack.push_back(std::make_unique<AtomSelectorNode_T<E, P>>(std::move(true_child), atom.get_atom()));
}

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
void accept_impl(ResultInverseTreeVisitor<E>& visitor, const InverseAtomSelectorNode_F<E, P>& atom)
{
    auto false_child = std::move(visitor.m_result_stack.back());
    visitor.m_result_stack.pop_back();
    // Construct new result
    visitor.m_result_stack.push_back(std::make_unique<AtomSelectorNode_F<E, P>>(std::move(false_child), atom.get_atom()));
}

template<formalism::HasConjunctiveCondition E>
void ResultInverseTreeVisitor<E>::accept(const InverseAtomSelectorNode_TFX<E, FluentTag>& atom)
{
    accept_impl(*this, atom);
}

template<formalism::HasConjunctiveCondition E>
void ResultInverseTreeVisitor<E>::accept(const InverseAtomSelectorNode_TF<E, FluentTag>& atom)
{
    accept_impl(*this, atom);
}

template<formalism::HasConjunctiveCondition E>
void ResultInverseTreeVisitor<E>::accept(const InverseAtomSelectorNode_TX<E, FluentTag>& atom)
{
    accept_impl(*this, atom);
}

template<formalism::HasConjunctiveCondition E>
void ResultInverseTreeVisitor<E>::accept(const InverseAtomSelectorNode_FX<E, FluentTag>& atom)
{
    accept_impl(*this, atom);
}

template<formalism::HasConjunctiveCondition E>
void ResultInverseTreeVisitor<E>::accept(const InverseAtomSelectorNode_T<E, FluentTag>& atom)
{
    accept_impl(*this, atom);
}

template<formalism::HasConjunctiveCondition E>
void ResultInverseTreeVisitor<E>::accept(const InverseAtomSelectorNode_F<E, FluentTag>& atom)
{
    accept_impl(*this, atom);
}

template<formalism::HasConjunctiveCondition E>
void ResultInverseTreeVisitor<E>::accept(const InverseAtomSelectorNode_TFX<E, DerivedTag>& atom)
{
    accept_impl(*this, atom);
}

template<formalism::HasConjunctiveCondition E>
void ResultInverseTreeVisitor<E>::accept(const InverseAtomSelectorNode_TF<E, DerivedTag>& atom)
{
    accept_impl(*this, atom);
}

template<formalism::HasConjunctiveCondition E>
void ResultInverseTreeVisitor<E>::accept(const InverseAtomSelectorNode_TX<E, DerivedTag>& atom)
{
    accept_impl(*this, atom);
}

template<formalism::HasConjunctiveCondition E>
void ResultInverseTreeVisitor<E>::accept(const InverseAtomSelectorNode_FX<E, DerivedTag>& atom)
{
    accept_impl(*this, atom);
}

template<formalism::HasConjunctiveCondition E>
void ResultInverseTreeVisitor<E>::accept(const InverseAtomSelectorNode_T<E, DerivedTag>& atom)
{
    accept_impl(*this, atom);
}

template<formalism::HasConjunctiveCondition E>
void ResultInverseTreeVisitor<E>::accept(const InverseAtomSelectorNode_F<E, DerivedTag>& atom)
{
    accept_impl(*this, atom);
}

template<formalism::HasConjunctiveCondition E>
void ResultInverseTreeVisitor<E>::accept(const InverseNumericConstraintSelectorNode_TX<E>& constraint)
{
    auto true_child = std::move(m_result_stack.back());
    m_result_stack.pop_back();
    auto dont_care_child = std::move(m_result_stack.back());
    m_result_stack.pop_back();
    // Construct new result
    m_result_stack.push_back(
        std::make_unique<NumericConstraintSelectorNode_TX<E>>(std::move(true_child), std::move(dont_care_child), constraint.get_constraint()));
}

template<formalism::HasConjunctiveCondition E>
void ResultInverseTreeVisitor<E>::accept(const InverseNumericConstraintSelectorNode_T<E>& constraint)
{
    auto true_child = std::move(m_result_stack.back());
    m_result_stack.pop_back();
    // Construct new result
    m_result_stack.push_back(std::make_unique<NumericConstraintSelectorNode_T<E>>(std::move(true_child), constraint.get_constraint()));
}

template<formalism::HasConjunctiveCondition E>
void ResultInverseTreeVisitor<E>::accept(const InverseElementGeneratorNode_Perfect<E>& generator)
{
    assert(!generator.get_elements().empty());
    m_result_stack.push_back(std::make_unique<ElementGeneratorNode_Perfect<E>>(generator.get_elements()));
}

template<formalism::HasConjunctiveCondition E>
void ResultInverseTreeVisitor<E>::accept(const InverseElementGeneratorNode_Imperfect<E>& generator)
{
    assert(!generator.get_elements().empty());
    m_result_stack.push_back(std::make_unique<ElementGeneratorNode_Imperfect<E>>(generator.get_elements()));
}

template<formalism::HasConjunctiveCondition E>
struct ExpandInverseTreeVisitor : public IInverseNodeVisitor<E>
{
    std::vector<NodeCreationStackEntry<E>>& m_stack;

    ExpandInverseTreeVisitor(std::vector<NodeCreationStackEntry<E>>& stack) : m_stack(stack) {}

    void accept(const InverseAtomSelectorNode_TFX<E, FluentTag>& atom) override;
    void accept(const InverseAtomSelectorNode_TF<E, FluentTag>& atom) override;
    void accept(const InverseAtomSelectorNode_TX<E, FluentTag>& atom) override;
    void accept(const InverseAtomSelectorNode_FX<E, FluentTag>& atom) override;
    void accept(const InverseAtomSelectorNode_T<E, FluentTag>& atom) override;
    void accept(const InverseAtomSelectorNode_F<E, FluentTag>& atom) override;
    void accept(const InverseAtomSelectorNode_TFX<E, DerivedTag>& atom) override;
    void accept(const InverseAtomSelectorNode_TF<E, DerivedTag>& atom) override;
    void accept(const InverseAtomSelectorNode_TX<E, DerivedTag>& atom) override;
    void accept(const InverseAtomSelectorNode_FX<E, DerivedTag>& atom) override;
    void accept(const InverseAtomSelectorNode_T<E, DerivedTag>& atom) override;
    void accept(const InverseAtomSelectorNode_F<E, DerivedTag>& atom) override;
    void accept(const InverseNumericConstraintSelectorNode_T<E>& constraint) override;
    void accept(const InverseNumericConstraintSelectorNode_TX<E>& constraint) override;
    void accept(const InverseElementGeneratorNode_Perfect<E>& generator) override;
    void accept(const InverseElementGeneratorNode_Imperfect<E>& generator) override;
};

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
void accept_impl(ExpandInverseTreeVisitor<E>& visitor, const InverseAtomSelectorNode_TFX<E, P>& atom)
{
    visitor.m_stack.push_back(NodeCreationStackEntry<E> { false, atom.get_true_child().get() });
    visitor.m_stack.push_back(NodeCreationStackEntry<E> { false, atom.get_false_child().get() });
    visitor.m_stack.push_back(NodeCreationStackEntry<E> { false, atom.get_dontcare_child().get() });
}

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
void accept_impl(ExpandInverseTreeVisitor<E>& visitor, const InverseAtomSelectorNode_TF<E, P>& atom)
{
    visitor.m_stack.push_back(NodeCreationStackEntry<E> { false, atom.get_true_child().get() });
    visitor.m_stack.push_back(NodeCreationStackEntry<E> { false, atom.get_false_child().get() });
}

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
void accept_impl(ExpandInverseTreeVisitor<E>& visitor, const InverseAtomSelectorNode_TX<E, P>& atom)
{
    visitor.m_stack.push_back(NodeCreationStackEntry<E> { false, atom.get_true_child().get() });
    visitor.m_stack.push_back(NodeCreationStackEntry<E> { false, atom.get_dontcare_child().get() });
}

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
void accept_impl(ExpandInverseTreeVisitor<E>& visitor, const InverseAtomSelectorNode_FX<E, P>& atom)
{
    visitor.m_stack.push_back(NodeCreationStackEntry<E> { false, atom.get_false_child().get() });
    visitor.m_stack.push_back(NodeCreationStackEntry<E> { false, atom.get_dontcare_child().get() });
}

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
void accept_impl(ExpandInverseTreeVisitor<E>& visitor, const InverseAtomSelectorNode_T<E, P>& atom)
{
    visitor.m_stack.push_back(NodeCreationStackEntry<E> { false, atom.get_true_child().get() });
}

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
void accept_impl(ExpandInverseTreeVisitor<E>& visitor, const InverseAtomSelectorNode_F<E, P>& atom)
{
    visitor.m_stack.push_back(NodeCreationStackEntry<E> { false, atom.get_false_child().get() });
}

template<formalism::HasConjunctiveCondition E>
void ExpandInverseTreeVisitor<E>::accept(const InverseAtomSelectorNode_TFX<E, FluentTag>& atom)
{
    accept_impl(*this, atom);
}

template<formalism::HasConjunctiveCondition E>
void ExpandInverseTreeVisitor<E>::accept(const InverseAtomSelectorNode_TF<E, FluentTag>& atom)
{
    accept_impl(*this, atom);
}

template<formalism::HasConjunctiveCondition E>
void ExpandInverseTreeVisitor<E>::accept(const InverseAtomSelectorNode_TX<E, FluentTag>& atom)
{
    accept_impl(*this, atom);
}

template<formalism::HasConjunctiveCondition E>
void ExpandInverseTreeVisitor<E>::accept(const InverseAtomSelectorNode_FX<E, FluentTag>& atom)
{
    accept_impl(*this, atom);
}

template<formalism::HasConjunctiveCondition E>
void ExpandInverseTreeVisitor<E>::accept(const InverseAtomSelectorNode_T<E, FluentTag>& atom)
{
    accept_impl(*this, atom);
}

template<formalism::HasConjunctiveCondition E>
void ExpandInverseTreeVisitor<E>::accept(const InverseAtomSelectorNode_F<E, FluentTag>& atom)
{
    accept_impl(*this, atom);
}

template<formalism::HasConjunctiveCondition E>
void ExpandInverseTreeVisitor<E>::accept(const InverseAtomSelectorNode_TFX<E, DerivedTag>& atom)
{
    accept_impl(*this, atom);
}

template<formalism::HasConjunctiveCondition E>
void ExpandInverseTreeVisitor<E>::accept(const InverseAtomSelectorNode_TF<E, DerivedTag>& atom)
{
    accept_impl(*this, atom);
}

template<formalism::HasConjunctiveCondition E>
void ExpandInverseTreeVisitor<E>::accept(const InverseAtomSelectorNode_TX<E, DerivedTag>& atom)
{
    accept_impl(*this, atom);
}

template<formalism::HasConjunctiveCondition E>
void ExpandInverseTreeVisitor<E>::accept(const InverseAtomSelectorNode_FX<E, DerivedTag>& atom)
{
    accept_impl(*this, atom);
}

template<formalism::HasConjunctiveCondition E>
void ExpandInverseTreeVisitor<E>::accept(const InverseAtomSelectorNode_T<E, DerivedTag>& atom)
{
    accept_impl(*this, atom);
}

template<formalism::HasConjunctiveCondition E>
void ExpandInverseTreeVisitor<E>::accept(const InverseAtomSelectorNode_F<E, DerivedTag>& atom)
{
    accept_impl(*this, atom);
}

template<formalism::HasConjunctiveCondition E>
void ExpandInverseTreeVisitor<E>::accept(const InverseNumericConstraintSelectorNode_TX<E>& constraint)
{
    m_stack.push_back(NodeCreationStackEntry<E> { false, constraint.get_true_child().get() });
    m_stack.push_back(NodeCreationStackEntry<E> { false, constraint.get_dontcare_child().get() });
}

template<formalism::HasConjunctiveCondition E>
void ExpandInverseTreeVisitor<E>::accept(const InverseNumericConstraintSelectorNode_T<E>& constraint)
{
    m_stack.push_back(NodeCreationStackEntry<E> { false, constraint.get_true_child().get() });
}

template<formalism::HasConjunctiveCondition E>
void ExpandInverseTreeVisitor<E>::accept(const InverseElementGeneratorNode_Perfect<E>& generator)
{
    // Nothing to be done
}

template<formalism::HasConjunctiveCondition E>
void ExpandInverseTreeVisitor<E>::accept(const InverseElementGeneratorNode_Imperfect<E>& generator)
{
    // Nothing to be done
}

template<formalism::HasConjunctiveCondition E>
Node<E> parse_inverse_tree_iteratively(const InverseNode<E>& root)
{
    auto stack = std::vector<NodeCreationStackEntry<E>> { NodeCreationStackEntry<E> { false, root.get() } };
    auto result_stack = std::vector<Node<E>> {};

    while (!stack.empty())
    {
        auto& entry = stack.back();
        const auto cur_node = entry.node;  // must copy because entry might be invalidated during visitation.

        if (!entry.expanded)
        {
            // Expand the node
            entry.expanded = true;  // must set this first because entry might be invalidated during visitation.

            auto visitor = ExpandInverseTreeVisitor<E>(stack);
            cur_node->visit(visitor);
        }
        else
        {
            // compute the result into the result_stack.

            stack.pop_back();  ///< We can already pop here to avoid duplication in the visitor.

            auto visitor = ResultInverseTreeVisitor<E>(stack, result_stack);
            cur_node->visit(visitor);
        }
    }
    // Only the root node should be in the result stack.
    assert(result_stack.size() == 1);

    // std::cout << std::make_tuple(std::cref(result_stack.back()), DotPrinterTag {}) << std::endl;

    return std::move(result_stack.back());
}

template Node<GroundActionImpl> parse_inverse_tree_iteratively(const InverseNode<GroundActionImpl>& root);
template Node<GroundAxiomImpl> parse_inverse_tree_iteratively(const InverseNode<GroundAxiomImpl>& root);

template<formalism::HasConjunctiveCondition E>
Node<E> create_root_generator_node(std::span<const E*> elements)
{
    return std::make_unique<ElementGeneratorNode_Perfect<E>>(elements);
}

template Node<GroundActionImpl> create_root_generator_node(std::span<const GroundActionImpl*> elements);
template Node<GroundAxiomImpl> create_root_generator_node(std::span<const GroundAxiomImpl*> elements);
}
