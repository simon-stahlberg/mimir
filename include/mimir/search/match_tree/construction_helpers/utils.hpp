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

#ifndef MIMIR_SEARCH_MATCH_TREE_CONSTRUCTION_HELPERS_UTILS_HPP_
#define MIMIR_SEARCH_MATCH_TREE_CONSTRUCTION_HELPERS_UTILS_HPP_

#include "mimir/formalism/ground_action.hpp"
#include "mimir/formalism/ground_axiom.hpp"
#include "mimir/search/match_tree/construction_helpers/inverse_nodes/atom.hpp"
#include "mimir/search/match_tree/construction_helpers/inverse_nodes/generator.hpp"
#include "mimir/search/match_tree/construction_helpers/inverse_nodes/numeric_constraint.hpp"
#include "mimir/search/match_tree/construction_helpers/split.hpp"
#include "mimir/search/match_tree/declarations.hpp"
#include "mimir/search/match_tree/nodes/atom.hpp"
#include "mimir/search/match_tree/nodes/generator.hpp"
#include "mimir/search/match_tree/nodes/numeric_constraint.hpp"
#include "mimir/search/match_tree/split_scoring_functions/interface.hpp"

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
InverseNode<Element>
create_node_from_split(AtomSplit<P> split, std::span<const Element*> elements, InverseNode<Element> parent, const SplitList& useless_splits)
{
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
    std::cout << "num_true: " << num_true;

    auto num_false = size_t(0);
    for (size_t i = num_true; i < elements.size(); ++i)
    {
        const auto& element = elements[i];
        if (contains_negative(atom, element))
        {
            std::swap(elements[i], elements[num_false++]);
        }
    }
    std::cout << "num_false: " << num_false;

    return std::make_shared<InverseAtomSelectorNode<Element, P>>(
        parent,
        useless_splits,
        0,
        0.,
        atom,
        std::span<const Element*>(elements.begin(), elements.begin() + num_true),
        std::span<const Element*>(elements.begin() + num_true, elements.begin() + num_true + num_false),
        std::span<const Element*>(elements.begin() + num_true + num_false, elements.end()));
}

template<HasConjunctiveCondition Element>
InverseNode<Element>
create_node_from_split(NumericConstraintSplit split, std::span<const Element*> elements, InverseNode<Element> parent, const SplitList& useless_splits)
{
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
    std::cout << "num_true: " << num_true;

    return std::make_shared<InverseNumericConstraintSelectorNode<Element>>(parent,
                                                                           useless_splits,
                                                                           size_t(0),
                                                                           double(0.),
                                                                           constraint,
                                                                           std::span<const Element*>(elements.begin(), elements.begin() + num_true),
                                                                           std::span<const Element*>(elements.begin() + num_true, elements.end()));
}

template<HasConjunctiveCondition Element>
InverseNode<Element> create_node_from_split(const SplitScoringFunctionResult& split_result, std::span<const Element*> elements, InverseNode<Element> parent)
{
    return std::visit([&](auto&& arg) { return create_node_from_split(arg, elements, parent, split_result.useless_splits); }, split_result.split);
}
}

#endif
