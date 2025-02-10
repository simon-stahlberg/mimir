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

#ifndef MIMIR_SEARCH_MATCH_TREE_DECLARATIONS_HPP_
#define MIMIR_SEARCH_MATCH_TREE_DECLARATIONS_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/search/declarations.hpp"

#include <concepts>
#include <variant>
#include <vector>

namespace mimir::match_tree
{

/**
 * Concepts
 */

/// MatchTree can handle elements that have a conjunctive condition, e.g., GroundActionImpl and GroundAxiomImpl.
template<typename T>
concept HasConjunctiveCondition = requires(const T a) {
    { a.get_conjunctive_condition() } -> std::same_as<const GroundConjunctiveCondition&>;
};

/**
 * Forward declarations
 */

template<HasConjunctiveCondition Element>
class IInverseNode;
template<HasConjunctiveCondition Element, DynamicPredicateTag P>
class InverseAtomSelectorNode_TFX;
template<HasConjunctiveCondition Element, DynamicPredicateTag P>
class InverseAtomSelectorNode_TF;
template<HasConjunctiveCondition Element, DynamicPredicateTag P>
class InverseAtomSelectorNode_TX;
template<HasConjunctiveCondition Element, DynamicPredicateTag P>
class InverseAtomSelectorNode_FX;
template<HasConjunctiveCondition Element, DynamicPredicateTag P>
class InverseAtomSelectorNode_T;
template<HasConjunctiveCondition Element, DynamicPredicateTag P>
class InverseAtomSelectorNode_F;
template<HasConjunctiveCondition Element>
class InverseNumericConstraintSelectorNode;
template<HasConjunctiveCondition Element>
class InverseElementGeneratorNode;

template<HasConjunctiveCondition Element>
class PlaceholderNodeImpl;
template<HasConjunctiveCondition Element>
using PlaceholderNode = std::unique_ptr<PlaceholderNodeImpl<Element>>;
template<HasConjunctiveCondition Element>
using PlaceholderNodeList = std::vector<PlaceholderNode<Element>>;

template<HasConjunctiveCondition Element>
class INode;
template<HasConjunctiveCondition Element, DynamicPredicateTag P>
class AtomSelectorNode_TFX;
template<HasConjunctiveCondition Element, DynamicPredicateTag P>
class AtomSelectorNode_TF;
template<HasConjunctiveCondition Element, DynamicPredicateTag P>
class AtomSelectorNode_TX;
template<HasConjunctiveCondition Element, DynamicPredicateTag P>
class AtomSelectorNode_FX;
template<HasConjunctiveCondition Element, DynamicPredicateTag P>
class AtomSelectorNode_T;
template<HasConjunctiveCondition Element, DynamicPredicateTag P>
class AtomSelectorNode_F;
template<HasConjunctiveCondition Element>
class NumericConstraintSelectorNode;
template<HasConjunctiveCondition Element>
class ElementGeneratorNode;

template<HasConjunctiveCondition Element>
class INodeScoreFunction;

template<HasConjunctiveCondition Element>
class INodeSplitter;

/**
 * Aliases
 */

template<HasConjunctiveCondition Element>
using InverseNode = std::unique_ptr<IInverseNode<Element>>;
template<HasConjunctiveCondition Element>
using InverseNodeList = std::vector<InverseNode<Element>>;

template<HasConjunctiveCondition Element>
using Node = std::unique_ptr<INode<Element>>;

template<HasConjunctiveCondition Element>
using NodeScoreFunction = std::unique_ptr<INodeScoreFunction<Element>>;

template<HasConjunctiveCondition Element>
using NodeSplitter = std::unique_ptr<INodeSplitter<Element>>;
}

#endif
