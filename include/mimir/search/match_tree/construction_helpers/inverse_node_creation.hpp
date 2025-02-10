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

#ifndef MIMIR_SEARCH_MATCH_TREE_CONSTRUCTION_HELPERS_INVERSE_NODE_CREATION_HPP_
#define MIMIR_SEARCH_MATCH_TREE_CONSTRUCTION_HELPERS_INVERSE_NODE_CREATION_HPP_

#include "mimir/formalism/ground_action.hpp"
#include "mimir/formalism/ground_axiom.hpp"
#include "mimir/search/match_tree/construction_helpers/inverse_nodes/atom.hpp"
#include "mimir/search/match_tree/construction_helpers/inverse_nodes/generator.hpp"
#include "mimir/search/match_tree/construction_helpers/inverse_nodes/numeric_constraint.hpp"
#include "mimir/search/match_tree/construction_helpers/inverse_nodes/placeholder.hpp"
#include "mimir/search/match_tree/construction_helpers/split.hpp"
#include "mimir/search/match_tree/declarations.hpp"
#include "mimir/search/match_tree/nodes/atom.hpp"
#include "mimir/search/match_tree/nodes/generator.hpp"
#include "mimir/search/match_tree/nodes/numeric_constraint.hpp"

#include <optional>
#include <queue>
#include <vector>

namespace mimir::match_tree
{
template<HasConjunctiveCondition Element, DynamicPredicateTag P>
bool contains_positive(GroundAtom<P> atom, const Element* element)
{
    const auto& conjunctive_condition = element->get_conjunctive_condition();
    const auto& positive_precondition = conjunctive_condition.template get_positive_precondition<P>();
    return (std::find(positive_precondition.begin(), positive_precondition.end(), atom->get_index()) != positive_precondition.end());
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
bool contains_negative(GroundAtom<P> atom, const Element* element)
{
    const auto& conjunctive_condition = element->get_conjunctive_condition();
    const auto& negative_precondition = conjunctive_condition.template get_negative_precondition<P>();
    return (std::find(negative_precondition.begin(), negative_precondition.end(), atom->get_index()) != negative_precondition.end());
}

template<HasConjunctiveCondition Element>
bool contains(GroundNumericConstraint constraint, const Element* element)
{
    const auto& conjunctive_condition = element->get_conjunctive_condition();
    const auto& numeric_constraints = conjunctive_condition.get_numeric_constraints();
    return (std::find(numeric_constraints.begin(), numeric_constraints.end(), constraint) != numeric_constraints.end());
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
std::optional<std::pair<InverseNode<Element>, PlaceholderNodeList<Element>>>
create_node_and_placeholder_children(const PlaceholderNode<Element>& node, const SplitList& useless_splits, size_t root_distance, AtomSplit<P> split)
{
    assert(node);

    const auto elements = node->get_elements();
    const auto atom = split.feature;

    // swap true to front
    auto num_true = size_t(0);
    for (size_t i = 0; i < elements.size(); ++i)
    {
        const auto& element = elements[i];
        if (contains_positive(atom, element))
        {
            std::swap(elements[i], elements[num_true++]);
        }
    }

    auto num_false = size_t(0);
    for (size_t i = num_true; i < elements.size(); ++i)
    {
        const auto& element = elements[i];
        if (contains_negative(atom, element))
        {
            std::swap(elements[i], elements[num_false++]);
        }
    }

    const auto num_dontcare = elements.size() - num_true - num_false;

    if ((num_true == 0 && num_false == 0) || (num_true == 0 && num_dontcare == 0) || (num_false == 0 && num_dontcare == 0))
    {
        return std::nullopt;  ///< Avoid creating useless nodes
    }

    auto true_elements = std::span<const Element*>(elements.begin(), elements.begin() + num_true);
    auto false_elements = std::span<const Element*>(elements.begin() + num_true, elements.begin() + num_true + num_false);
    auto dontcare_elements = std::span<const Element*>(elements.begin() + num_true + num_false, elements.end());

    if (node->get_parent())
    {
        /* Construct the node directly into the parents child and return nullptr, i.e., it is an inner node. */
        auto& created_node = node->get_parents_child() = std::make_unique<InverseAtomSelectorNode<Element, P>>(node->get_parent(),
                                                                                                               useless_splits,
                                                                                                               node->get_root_distance(),
                                                                                                               atom,
                                                                                                               true_elements,
                                                                                                               false_elements,
                                                                                                               dontcare_elements);
        auto atom_node = dynamic_cast<InverseAtomSelectorNode<Element, P>*>(created_node.get());
        assert(atom_node);

        auto children = PlaceholderNodeList<Element> {};
        children.push_back(std::make_unique<PlaceholderNodeImpl<Element>>(created_node.get(), atom_node->get_true_child(), root_distance, true_elements));
        children.push_back(std::make_unique<PlaceholderNodeImpl<Element>>(created_node.get(), atom_node->get_false_child(), root_distance, false_elements));
        children.push_back(
            std::make_unique<PlaceholderNodeImpl<Element>>(created_node.get(), atom_node->get_dontcare_child(), root_distance, dontcare_elements));

        return std::make_pair(InverseNode<Element> { nullptr }, std::move(children));
    }
    else
    {
        /* Construct the node and return it, i.e., the root node. */
        auto created_node = std::make_unique<InverseAtomSelectorNode<Element, P>>(nullptr,
                                                                                  useless_splits,
                                                                                  node->get_root_distance(),
                                                                                  atom,
                                                                                  true_elements,
                                                                                  false_elements,
                                                                                  dontcare_elements);

        auto children = PlaceholderNodeList<Element> {};
        children.push_back(std::make_unique<PlaceholderNodeImpl<Element>>(created_node.get(), created_node->get_true_child(), root_distance, true_elements));
        children.push_back(std::make_unique<PlaceholderNodeImpl<Element>>(created_node.get(), created_node->get_false_child(), root_distance, false_elements));
        children.push_back(
            std::make_unique<PlaceholderNodeImpl<Element>>(created_node.get(), created_node->get_dontcare_child(), root_distance, dontcare_elements));

        return std::make_pair(std::move(created_node), std::move(children));
    }
}

template<HasConjunctiveCondition Element>
std::optional<std::pair<InverseNode<Element>, PlaceholderNodeList<Element>>>
create_node_and_placeholder_children(const PlaceholderNode<Element>& node, const SplitList& useless_splits, size_t root_distance, NumericConstraintSplit split)
{
    assert(node);

    const auto elements = node->get_elements();
    const auto constraint = split.feature;

    // swap true to front
    auto num_true = size_t(0);
    for (size_t i = 0; i < elements.size(); ++i)
    {
        const auto& element = elements[i];
        if (contains(constraint, element))
        {
            std::swap(elements[i], elements[num_true++]);
        }
    }

    const auto num_dontcare = elements.size() - num_true;

    if (num_true == 0 || num_dontcare == 0)
    {
        return std::nullopt;  ///< Avoid creating useless nodes
    }

    auto true_elements = std::span<const Element*>(elements.begin(), elements.begin() + num_true);
    auto dontcare_elements = std::span<const Element*>(elements.begin() + num_true, elements.end());

    if (node->get_parent())
    {
        /* Construct the node directly into the parents child and return nullptr, i.e., it is an inner node. */
        auto& created_node = node->get_parents_child() = std::make_unique<InverseNumericConstraintSelectorNode<Element>>(node->get_parent(),
                                                                                                                         useless_splits,
                                                                                                                         node->get_root_distance(),
                                                                                                                         constraint,
                                                                                                                         true_elements,
                                                                                                                         dontcare_elements);

        auto constraint_node = dynamic_cast<InverseNumericConstraintSelectorNode<Element>*>(created_node.get());
        assert(constraint_node);

        auto children = PlaceholderNodeList<Element> {};
        children.push_back(std::make_unique<PlaceholderNodeImpl<Element>>(created_node.get(), constraint_node->get_true_child(), root_distance, true_elements));
        children.push_back(
            std::make_unique<PlaceholderNodeImpl<Element>>(created_node.get(), constraint_node->get_dontcare_child(), root_distance, dontcare_elements));

        return std::make_pair(InverseNode<Element> { nullptr }, std::move(children));
    }
    else
    {
        /* Construct the node and return it, i.e., the root node. */
        auto created_node = std::make_unique<InverseNumericConstraintSelectorNode<Element>>(nullptr,
                                                                                            useless_splits,
                                                                                            node->get_root_distance(),
                                                                                            constraint,
                                                                                            true_elements,
                                                                                            dontcare_elements);

        auto children = PlaceholderNodeList<Element> {};
        children.push_back(std::make_unique<PlaceholderNodeImpl<Element>>(created_node.get(), created_node->get_true_child(), root_distance, true_elements));
        children.push_back(
            std::make_unique<PlaceholderNodeImpl<Element>>(created_node.get(), created_node->get_dontcare_child(), root_distance, dontcare_elements));

        return std::make_pair(std::move(created_node), std::move(children));
    }
}

template<HasConjunctiveCondition Element>
std::optional<std::pair<InverseNode<Element>, PlaceholderNodeList<Element>>>
create_node_and_placeholder_children(const PlaceholderNode<Element>& node, const SplitList& useless_splits, size_t root_distance, const Split& split)
{
    assert(node);

    return std::visit([&](auto&& arg) { return create_node_and_placeholder_children(node, useless_splits, root_distance, arg); }, split);
}

template<HasConjunctiveCondition Element>
InverseNode<Element> create_generator_node(const PlaceholderNode<Element>& node, size_t root_distance)
{
    assert(node);

    if (node->get_parent())
    {
        /* Construct the node directly into the parents child and return nullptr, i.e., it is an inner node. */
        node->get_parents_child() = std::make_unique<InverseElementGeneratorNode<Element>>(node->get_parent(), root_distance, node->get_elements());

        return nullptr;
    }
    else
    {
        /* Construct the node and return it, i.e., the root node. */
        return std::make_unique<InverseElementGeneratorNode<Element>>(node->get_parent(), root_distance, node->get_elements());
    }
}

template<HasConjunctiveCondition Element>
PlaceholderNode<Element> create_root_placeholder_node(std::span<const Element*> elements)
{
    return std::make_unique<PlaceholderNodeImpl<Element>>(nullptr, nullptr, 0, elements);
}

}

#endif
