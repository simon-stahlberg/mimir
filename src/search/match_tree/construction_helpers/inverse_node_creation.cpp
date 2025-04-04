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

#include "mimir/search/match_tree/construction_helpers/inverse_node_creation.hpp"

#include "mimir/formalism/ground_action.hpp"
#include "mimir/formalism/ground_axiom.hpp"
#include "mimir/formalism/predicate.hpp"
#include "mimir/search/match_tree/construction_helpers/inverse_nodes/atom.hpp"
#include "mimir/search/match_tree/construction_helpers/inverse_nodes/generator.hpp"
#include "mimir/search/match_tree/construction_helpers/inverse_nodes/numeric_constraint.hpp"
#include "mimir/search/match_tree/construction_helpers/inverse_nodes/placeholder.hpp"

#include <optional>
#include <queue>
#include <vector>

using namespace mimir::formalism;

namespace mimir::search::match_tree
{

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
std::pair<InverseNode<E>, PlaceholderNodeList<E>>
create_node_and_placeholder_children(const PlaceholderNode<E>& node, const SplitList& useless_splits, AtomSplit<P> split)
{
    assert(node);

    const auto elements = node->get_elements();
    const auto atom = split.feature;

    // swap true to front
    auto num_true = size_t(0);
    for (size_t i = 0; i < elements.size(); ++i)
    {
        if (contains_positive(atom, elements[i]))
        {
            std::swap(elements[num_true++], elements[i]);
        }
    }
    auto num_false = size_t(0);
    for (size_t i = num_true; i < elements.size(); ++i)
    {
        if (contains_negative(atom, elements[i]))
        {
            std::swap(elements[num_true + num_false++], elements[i]);
        }
    }

    assert(num_true != 0 || num_false != 0);

    auto true_elements = std::span<const E*>(elements.begin(), elements.begin() + num_true);
    auto false_elements = std::span<const E*>(elements.begin() + num_true, elements.begin() + num_true + num_false);
    auto dontcare_elements = std::span<const E*>(elements.begin() + num_true + num_false, elements.end());

    if (node->get_parent())
    {
        /* Construct the node directly into the parents child and return nullptr, i.e., it is an inner node. */

        if (!true_elements.empty() && !false_elements.empty() && !dontcare_elements.empty())
        {
            auto& created_node = node->get_parents_child() =
                std::make_unique<InverseAtomSelectorNode_TFX<E, P>>(node->get_parent(), useless_splits, atom, true_elements, false_elements, dontcare_elements);
            auto atom_node = dynamic_cast<InverseAtomSelectorNode_TFX<E, P>*>(created_node.get());
            assert(atom_node);

            auto children = PlaceholderNodeList<E> {};
            children.push_back(std::make_unique<PlaceholderNodeImpl<E>>(created_node.get(), &atom_node->get_true_child(), true_elements));
            children.push_back(std::make_unique<PlaceholderNodeImpl<E>>(created_node.get(), &atom_node->get_false_child(), false_elements));
            children.push_back(std::make_unique<PlaceholderNodeImpl<E>>(created_node.get(), &atom_node->get_dontcare_child(), dontcare_elements));

            return std::make_pair(InverseNode<E> { nullptr }, std::move(children));
        }
        else if (!true_elements.empty() && !false_elements.empty())
        {
            auto& created_node = node->get_parents_child() =
                std::make_unique<InverseAtomSelectorNode_TF<E, P>>(node->get_parent(), useless_splits, atom, true_elements, false_elements);
            auto atom_node = dynamic_cast<InverseAtomSelectorNode_TF<E, P>*>(created_node.get());
            assert(atom_node);

            auto children = PlaceholderNodeList<E> {};
            children.push_back(std::make_unique<PlaceholderNodeImpl<E>>(created_node.get(), &atom_node->get_true_child(), true_elements));
            children.push_back(std::make_unique<PlaceholderNodeImpl<E>>(created_node.get(), &atom_node->get_false_child(), false_elements));

            return std::make_pair(InverseNode<E> { nullptr }, std::move(children));
        }
        else if (!true_elements.empty() && !dontcare_elements.empty())
        {
            auto& created_node = node->get_parents_child() =
                std::make_unique<InverseAtomSelectorNode_TX<E, P>>(node->get_parent(), useless_splits, atom, true_elements, dontcare_elements);
            auto atom_node = dynamic_cast<InverseAtomSelectorNode_TX<E, P>*>(created_node.get());
            assert(atom_node);

            auto children = PlaceholderNodeList<E> {};
            children.push_back(std::make_unique<PlaceholderNodeImpl<E>>(created_node.get(), &atom_node->get_true_child(), true_elements));
            children.push_back(std::make_unique<PlaceholderNodeImpl<E>>(created_node.get(), &atom_node->get_dontcare_child(), dontcare_elements));

            return std::make_pair(InverseNode<E> { nullptr }, std::move(children));
        }
        else if (!false_elements.empty() && !dontcare_elements.empty())
        {
            auto& created_node = node->get_parents_child() =
                std::make_unique<InverseAtomSelectorNode_FX<E, P>>(node->get_parent(), useless_splits, atom, false_elements, dontcare_elements);
            auto atom_node = dynamic_cast<InverseAtomSelectorNode_FX<E, P>*>(created_node.get());
            assert(atom_node);

            auto children = PlaceholderNodeList<E> {};
            children.push_back(std::make_unique<PlaceholderNodeImpl<E>>(created_node.get(), &atom_node->get_false_child(), false_elements));
            children.push_back(std::make_unique<PlaceholderNodeImpl<E>>(created_node.get(), &atom_node->get_dontcare_child(), dontcare_elements));

            return std::make_pair(InverseNode<E> { nullptr }, std::move(children));
        }
        else if (!true_elements.empty())
        {
            auto& created_node = node->get_parents_child() =
                std::make_unique<InverseAtomSelectorNode_T<E, P>>(node->get_parent(), useless_splits, atom, true_elements);
            auto atom_node = dynamic_cast<InverseAtomSelectorNode_T<E, P>*>(created_node.get());
            assert(atom_node);

            auto children = PlaceholderNodeList<E> {};
            children.push_back(std::make_unique<PlaceholderNodeImpl<E>>(created_node.get(), &atom_node->get_true_child(), true_elements));

            return std::make_pair(InverseNode<E> { nullptr }, std::move(children));
        }
        else if (!false_elements.empty())
        {
            auto& created_node = node->get_parents_child() =
                std::make_unique<InverseAtomSelectorNode_F<E, P>>(node->get_parent(), useless_splits, atom, false_elements);
            auto atom_node = dynamic_cast<InverseAtomSelectorNode_F<E, P>*>(created_node.get());
            assert(atom_node);

            auto children = PlaceholderNodeList<E> {};
            children.push_back(std::make_unique<PlaceholderNodeImpl<E>>(created_node.get(), &atom_node->get_false_child(), false_elements));

            return std::make_pair(InverseNode<E> { nullptr }, std::move(children));
        }
        else
        {
            throw std::logic_error(
                "create_node_and_placeholder_children(node, useless_splits, root_distance, split): Tried to create useless inverse atom node.");
        }
    }
    else
    {
        /* Construct the node and return it, i.e., the root node. */
        if (!true_elements.empty() && !false_elements.empty() && !dontcare_elements.empty())
        {
            auto created_node =
                std::make_unique<InverseAtomSelectorNode_TFX<E, P>>(nullptr, useless_splits, atom, true_elements, false_elements, dontcare_elements);

            auto children = PlaceholderNodeList<E> {};
            children.push_back(std::make_unique<PlaceholderNodeImpl<E>>(created_node.get(), &created_node->get_true_child(), true_elements));
            children.push_back(std::make_unique<PlaceholderNodeImpl<E>>(created_node.get(), &created_node->get_false_child(), false_elements));
            children.push_back(std::make_unique<PlaceholderNodeImpl<E>>(created_node.get(), &created_node->get_dontcare_child(), dontcare_elements));

            return std::make_pair(std::move(created_node), std::move(children));
        }
        else if (!true_elements.empty() && !false_elements.empty())
        {
            auto created_node = std::make_unique<InverseAtomSelectorNode_TF<E, P>>(nullptr, useless_splits, atom, true_elements, false_elements);

            auto children = PlaceholderNodeList<E> {};
            children.push_back(std::make_unique<PlaceholderNodeImpl<E>>(created_node.get(), &created_node->get_true_child(), true_elements));
            children.push_back(std::make_unique<PlaceholderNodeImpl<E>>(created_node.get(), &created_node->get_false_child(), false_elements));

            return std::make_pair(std::move(created_node), std::move(children));
        }
        else if (!true_elements.empty() && !dontcare_elements.empty())
        {
            auto created_node = std::make_unique<InverseAtomSelectorNode_TX<E, P>>(nullptr, useless_splits, atom, true_elements, dontcare_elements);

            auto children = PlaceholderNodeList<E> {};
            children.push_back(std::make_unique<PlaceholderNodeImpl<E>>(created_node.get(), &created_node->get_true_child(), true_elements));
            children.push_back(std::make_unique<PlaceholderNodeImpl<E>>(created_node.get(), &created_node->get_dontcare_child(), dontcare_elements));

            return std::make_pair(std::move(created_node), std::move(children));
        }
        else if (!false_elements.empty() && !dontcare_elements.empty())
        {
            auto created_node = std::make_unique<InverseAtomSelectorNode_FX<E, P>>(nullptr, useless_splits, atom, false_elements, dontcare_elements);

            auto children = PlaceholderNodeList<E> {};
            children.push_back(std::make_unique<PlaceholderNodeImpl<E>>(created_node.get(), &created_node->get_false_child(), false_elements));
            children.push_back(std::make_unique<PlaceholderNodeImpl<E>>(created_node.get(), &created_node->get_dontcare_child(), dontcare_elements));

            return std::make_pair(std::move(created_node), std::move(children));
        }
        else if (!true_elements.empty())
        {
            auto created_node = std::make_unique<InverseAtomSelectorNode_T<E, P>>(nullptr, useless_splits, atom, true_elements);

            auto children = PlaceholderNodeList<E> {};
            children.push_back(std::make_unique<PlaceholderNodeImpl<E>>(created_node.get(), &created_node->get_true_child(), true_elements));

            return std::make_pair(std::move(created_node), std::move(children));
        }
        else if (!false_elements.empty())
        {
            auto created_node = std::make_unique<InverseAtomSelectorNode_F<E, P>>(nullptr, useless_splits, atom, false_elements);

            auto children = PlaceholderNodeList<E> {};
            children.push_back(std::make_unique<PlaceholderNodeImpl<E>>(created_node.get(), &created_node->get_false_child(), false_elements));

            return std::make_pair(std::move(created_node), std::move(children));
        }
        else
        {
            throw std::logic_error(
                "create_node_and_placeholder_children(node, useless_splits, root_distance, split): Tried to create useless inverse atom node.");
        }
    }
}

template std::pair<InverseNode<GroundActionImpl>, PlaceholderNodeList<GroundActionImpl>>
create_node_and_placeholder_children(const PlaceholderNode<GroundActionImpl>& node, const SplitList& useless_splits, AtomSplit<FluentTag> split);
template std::pair<InverseNode<GroundActionImpl>, PlaceholderNodeList<GroundActionImpl>>
create_node_and_placeholder_children(const PlaceholderNode<GroundActionImpl>& node, const SplitList& useless_splits, AtomSplit<DerivedTag> split);
template std::pair<InverseNode<GroundAxiomImpl>, PlaceholderNodeList<GroundAxiomImpl>>
create_node_and_placeholder_children(const PlaceholderNode<GroundAxiomImpl>& node, const SplitList& useless_splits, AtomSplit<FluentTag> split);
template std::pair<InverseNode<GroundAxiomImpl>, PlaceholderNodeList<GroundAxiomImpl>>
create_node_and_placeholder_children(const PlaceholderNode<GroundAxiomImpl>& node, const SplitList& useless_splits, AtomSplit<DerivedTag> split);

template<formalism::HasConjunctiveCondition E>
std::pair<InverseNode<E>, PlaceholderNodeList<E>>
create_node_and_placeholder_children(const PlaceholderNode<E>& node, const SplitList& useless_splits, NumericConstraintSplit split)
{
    assert(node);

    const auto elements = node->get_elements();
    const auto constraint = split.feature;

    // swap true to front
    auto num_true = size_t(0);
    for (size_t i = 0; i < elements.size(); ++i)
    {
        if (contains(constraint, elements[i]))
        {
            std::swap(elements[num_true++], elements[i]);
        }
    }

    assert(num_true != 0);

    auto true_elements = std::span<const E*>(elements.begin(), elements.begin() + num_true);
    auto dontcare_elements = std::span<const E*>(elements.begin() + num_true, elements.end());

    if (node->get_parent())
    {
        /* Construct the node directly into the parents child and return nullptr, i.e., it is an inner node. */

        if (!true_elements.empty() && !dontcare_elements.empty())
        {
            auto& created_node = node->get_parents_child() =
                std::make_unique<InverseNumericConstraintSelectorNode_TX<E>>(node->get_parent(), useless_splits, constraint, true_elements, dontcare_elements);

            auto constraint_node = dynamic_cast<InverseNumericConstraintSelectorNode_TX<E>*>(created_node.get());
            assert(constraint_node);

            auto children = PlaceholderNodeList<E> {};
            children.push_back(std::make_unique<PlaceholderNodeImpl<E>>(created_node.get(), &constraint_node->get_true_child(), true_elements));
            children.push_back(std::make_unique<PlaceholderNodeImpl<E>>(created_node.get(), &constraint_node->get_dontcare_child(), dontcare_elements));

            return std::make_pair(InverseNode<E> { nullptr }, std::move(children));
        }
        else if (!true_elements.empty())
        {
            auto& created_node = node->get_parents_child() =
                std::make_unique<InverseNumericConstraintSelectorNode_T<E>>(node->get_parent(), useless_splits, constraint, true_elements);

            auto constraint_node = dynamic_cast<InverseNumericConstraintSelectorNode_T<E>*>(created_node.get());
            assert(constraint_node);

            auto children = PlaceholderNodeList<E> {};
            children.push_back(std::make_unique<PlaceholderNodeImpl<E>>(created_node.get(), &constraint_node->get_true_child(), true_elements));

            return std::make_pair(InverseNode<E> { nullptr }, std::move(children));
        }
        else
        {
            throw std::logic_error(
                "create_node_and_placeholder_children(node, useless_splits, root_distance, split): Tried to create useless inverse constraint node.");
        }
    }
    else
    {
        /* Construct the node and return it, i.e., the root node. */

        if (!true_elements.empty() && !dontcare_elements.empty())
        {
            auto created_node =
                std::make_unique<InverseNumericConstraintSelectorNode_TX<E>>(nullptr, useless_splits, constraint, true_elements, dontcare_elements);

            auto children = PlaceholderNodeList<E> {};
            children.push_back(std::make_unique<PlaceholderNodeImpl<E>>(created_node.get(), &created_node->get_true_child(), true_elements));
            children.push_back(std::make_unique<PlaceholderNodeImpl<E>>(created_node.get(), &created_node->get_dontcare_child(), dontcare_elements));

            return std::make_pair(std::move(created_node), std::move(children));
        }
        else if (!true_elements.empty())
        {
            auto created_node = std::make_unique<InverseNumericConstraintSelectorNode_T<E>>(nullptr, useless_splits, constraint, true_elements);

            auto children = PlaceholderNodeList<E> {};
            children.push_back(std::make_unique<PlaceholderNodeImpl<E>>(created_node.get(), &created_node->get_true_child(), true_elements));

            return std::make_pair(std::move(created_node), std::move(children));
        }
        else
        {
            throw std::logic_error(
                "create_node_and_placeholder_children(node, useless_splits, root_distance, split): Tried to create useless inverse constraint node.");
        }
    }
}

template std::pair<InverseNode<GroundActionImpl>, PlaceholderNodeList<GroundActionImpl>>
create_node_and_placeholder_children(const PlaceholderNode<GroundActionImpl>& node, const SplitList& useless_splits, NumericConstraintSplit split);
template std::pair<InverseNode<GroundAxiomImpl>, PlaceholderNodeList<GroundAxiomImpl>>
create_node_and_placeholder_children(const PlaceholderNode<GroundAxiomImpl>& node, const SplitList& useless_splits, NumericConstraintSplit split);

template<formalism::HasConjunctiveCondition E>
std::pair<InverseNode<E>, PlaceholderNodeList<E>>
create_node_and_placeholder_children(const PlaceholderNode<E>& node, const SplitList& useless_splits, const Split& split)
{
    assert(node);

    return std::visit([&](auto&& arg) { return create_node_and_placeholder_children(node, useless_splits, arg); }, split);
}

template std::pair<InverseNode<GroundActionImpl>, PlaceholderNodeList<GroundActionImpl>>
create_node_and_placeholder_children(const PlaceholderNode<GroundActionImpl>& node, const SplitList& useless_splits, const Split& split);
template std::pair<InverseNode<GroundAxiomImpl>, PlaceholderNodeList<GroundAxiomImpl>>
create_node_and_placeholder_children(const PlaceholderNode<GroundAxiomImpl>& node, const SplitList& useless_splits, const Split& split);

template<formalism::HasConjunctiveCondition E>
std::pair<InverseNode<E>, PlaceholderNodeList<E>> create_perfect_generator_node(const PlaceholderNode<E>& node)
{
    assert(node);

    if (node->get_parent())
    {
        /* Construct the node directly into the parents child and return nullptr, i.e., it is an inner node. */
        if (!node->get_elements().empty())  ///< Skip creating an empty generator node
        {
            node->get_parents_child() = std::make_unique<InverseElementGeneratorNode_Perfect<E>>(node->get_parent(), node->get_elements());
        }
        return { nullptr, PlaceholderNodeList<E> {} };
    }
    else
    {
        /* Construct the node and return it, i.e., the root node. */
        return { std::make_unique<InverseElementGeneratorNode_Perfect<E>>(node->get_parent(), node->get_elements()), PlaceholderNodeList<E> {} };
    }
}

template std::pair<InverseNode<GroundActionImpl>, PlaceholderNodeList<GroundActionImpl>>
create_perfect_generator_node(const PlaceholderNode<GroundActionImpl>& node);
template std::pair<InverseNode<GroundAxiomImpl>, PlaceholderNodeList<GroundAxiomImpl>>
create_perfect_generator_node(const PlaceholderNode<GroundAxiomImpl>& node);

template<formalism::HasConjunctiveCondition E>
InverseNode<E> create_imperfect_generator_node(const PlaceholderNode<E>& node)
{
    assert(node);

    if (node->get_parent())
    {
        /* Construct the node directly into the parents child and return nullptr, i.e., it is an inner node. */
        if (!node->get_elements().empty())  ///< Skip creating an empty generator node
        {
            node->get_parents_child() = std::make_unique<InverseElementGeneratorNode_Imperfect<E>>(node->get_parent(), node->get_elements());
        }
        return nullptr;
    }
    else
    {
        /* Construct the node and return it, i.e., the root node. */
        return std::make_unique<InverseElementGeneratorNode_Imperfect<E>>(node->get_parent(), node->get_elements());
    }
}

template InverseNode<GroundActionImpl> create_imperfect_generator_node(const PlaceholderNode<GroundActionImpl>& node);
template InverseNode<GroundAxiomImpl> create_imperfect_generator_node(const PlaceholderNode<GroundAxiomImpl>& node);

template<formalism::HasConjunctiveCondition E>
PlaceholderNode<E> create_root_placeholder_node(std::span<const E*> elements)
{
    return std::make_unique<PlaceholderNodeImpl<E>>(nullptr, nullptr, elements);
}

template PlaceholderNode<GroundActionImpl> create_root_placeholder_node(std::span<const GroundActionImpl*> elements);
template PlaceholderNode<GroundAxiomImpl> create_root_placeholder_node(std::span<const GroundAxiomImpl*> elements);

}
