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

namespace mimir::search::match_tree
{

/**
 * Concepts
 */

/// MatchTree can handle elements that have a conjunctive condition, e.g., GroundActionImpl and GroundAxiomImpl.
template<typename T>
concept HasConjunctiveCondition = requires(const T a) {
    { a.get_conjunctive_condition() } -> std::same_as<const formalism::GroundConjunctiveCondition&>;
};

/**
 * Forward declarations
 */

template<HasConjunctiveCondition E>
class PlaceholderNodeImpl;

template<HasConjunctiveCondition E>
class IInverseNode;
template<HasConjunctiveCondition E, formalism::IsFluentOrDerivedTag P>
class InverseAtomSelectorNode_TFX;
template<HasConjunctiveCondition E, formalism::IsFluentOrDerivedTag P>
class InverseAtomSelectorNode_TF;
template<HasConjunctiveCondition E, formalism::IsFluentOrDerivedTag P>
class InverseAtomSelectorNode_TX;
template<HasConjunctiveCondition E, formalism::IsFluentOrDerivedTag P>
class InverseAtomSelectorNode_FX;
template<HasConjunctiveCondition E, formalism::IsFluentOrDerivedTag P>
class InverseAtomSelectorNode_T;
template<HasConjunctiveCondition E, formalism::IsFluentOrDerivedTag P>
class InverseAtomSelectorNode_F;
template<HasConjunctiveCondition E>
class InverseNumericConstraintSelectorNode_TX;
template<HasConjunctiveCondition E>
class InverseNumericConstraintSelectorNode_T;
template<HasConjunctiveCondition E>
class InverseElementGeneratorNode_Perfect;
template<HasConjunctiveCondition E>
class InverseElementGeneratorNode_Imperfect;

template<HasConjunctiveCondition E>
class INode;
template<HasConjunctiveCondition E, formalism::IsFluentOrDerivedTag P>
class AtomSelectorNode_TFX;
template<HasConjunctiveCondition E, formalism::IsFluentOrDerivedTag P>
class AtomSelectorNode_TF;
template<HasConjunctiveCondition E, formalism::IsFluentOrDerivedTag P>
class AtomSelectorNode_TX;
template<HasConjunctiveCondition E, formalism::IsFluentOrDerivedTag P>
class AtomSelectorNode_FX;
template<HasConjunctiveCondition E, formalism::IsFluentOrDerivedTag P>
class AtomSelectorNode_T;
template<HasConjunctiveCondition E, formalism::IsFluentOrDerivedTag P>
class AtomSelectorNode_F;
template<HasConjunctiveCondition E>
class NumericConstraintSelectorNode_TX;
template<HasConjunctiveCondition E>
class NumericConstraintSelectorNode_T;
template<HasConjunctiveCondition E>
class ElementGeneratorNode_Perfect;
template<HasConjunctiveCondition E>
class ElementGeneratorNode_Imperfect;

template<HasConjunctiveCondition E>
class INodeScoreFunction;

template<HasConjunctiveCondition E>
class INodeSplitter;

struct Options;
struct Statistics;

/**
 * Aliases
 */

template<HasConjunctiveCondition E>
using PlaceholderNode = std::unique_ptr<PlaceholderNodeImpl<E>>;
template<HasConjunctiveCondition E>
using PlaceholderNodeList = std::vector<PlaceholderNode<E>>;

template<HasConjunctiveCondition E>
using InverseNode = std::unique_ptr<IInverseNode<E>>;
template<HasConjunctiveCondition E>
using InverseNodeList = std::vector<InverseNode<E>>;

template<HasConjunctiveCondition E>
using Node = std::unique_ptr<INode<E>>;

template<HasConjunctiveCondition E>
using NodeScoreFunction = std::unique_ptr<INodeScoreFunction<E>>;

template<HasConjunctiveCondition E>
using NodeSplitter = std::unique_ptr<INodeSplitter<E>>;
}

#endif
