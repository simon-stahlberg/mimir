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

#include "mimir/formalism/ground_atom.hpp"
#include "mimir/search/match_tree/construction_helpers/split.hpp"
#include "mimir/search/match_tree/declarations.hpp"

#include <optional>
#include <queue>
#include <vector>

namespace mimir::search::match_tree
{
template<formalism::HasConjunctiveCondition E, formalism::IsFluentOrDerivedTag P>
bool contains_positive(formalism::GroundAtom<P> atom, const E* element)
{
    const auto& conjunctive_condition = element->get_conjunctive_condition();
    const auto& positive_precondition = conjunctive_condition.template get_positive_precondition<P>();
    return (std::find(positive_precondition.begin(), positive_precondition.end(), atom->get_index()) != positive_precondition.end());
}

template<formalism::HasConjunctiveCondition E, formalism::IsFluentOrDerivedTag P>
bool contains_negative(formalism::GroundAtom<P> atom, const E* element)
{
    const auto& conjunctive_condition = element->get_conjunctive_condition();
    const auto& negative_precondition = conjunctive_condition.template get_negative_precondition<P>();
    return (std::find(negative_precondition.begin(), negative_precondition.end(), atom->get_index()) != negative_precondition.end());
}

template<formalism::HasConjunctiveCondition E>
bool contains(formalism::GroundNumericConstraint constraint, const E* element)
{
    const auto& conjunctive_condition = element->get_conjunctive_condition();
    const auto& numeric_constraints = conjunctive_condition.get_numeric_constraints();
    return (std::find(numeric_constraints.begin(), numeric_constraints.end(), constraint) != numeric_constraints.end());
}

template<formalism::HasConjunctiveCondition E, formalism::IsFluentOrDerivedTag P>
std::pair<InverseNode<E>, PlaceholderNodeList<E>> extern create_node_and_placeholder_children(const PlaceholderNode<E>& node,
                                                                                              const SplitList& useless_splits,
                                                                                              AtomSplit<P> split);

template<formalism::HasConjunctiveCondition E>
std::pair<InverseNode<E>, PlaceholderNodeList<E>> extern create_node_and_placeholder_children(const PlaceholderNode<E>& node,
                                                                                              const SplitList& useless_splits,
                                                                                              NumericConstraintSplit split);

template<formalism::HasConjunctiveCondition E>
std::pair<InverseNode<E>, PlaceholderNodeList<E>> extern create_node_and_placeholder_children(const PlaceholderNode<E>& node,
                                                                                              const SplitList& useless_splits,
                                                                                              const Split& split);

template<formalism::HasConjunctiveCondition E>
extern std::pair<InverseNode<E>, PlaceholderNodeList<E>> create_perfect_generator_node(const PlaceholderNode<E>& node);

template<formalism::HasConjunctiveCondition E>
extern InverseNode<E> create_imperfect_generator_node(const PlaceholderNode<E>& node);

template<formalism::HasConjunctiveCondition E>
extern PlaceholderNode<E> create_root_placeholder_node(std::span<const E*> elements);

}

#endif
