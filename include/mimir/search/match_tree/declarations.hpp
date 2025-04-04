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

#include "mimir/formalism/concepts.hpp"
#include "mimir/formalism/tags.hpp"

#include <variant>
#include <vector>

namespace mimir::search::match_tree
{

/**
 * Forward declarations
 */

template<formalism::HasConjunctiveCondition E>
class PlaceholderNodeImpl;

template<formalism::HasConjunctiveCondition E>
class IInverseNode;
template<formalism::HasConjunctiveCondition E, formalism::IsFluentOrDerivedTag P>
class InverseAtomSelectorNode_TFX;
template<formalism::HasConjunctiveCondition E, formalism::IsFluentOrDerivedTag P>
class InverseAtomSelectorNode_TF;
template<formalism::HasConjunctiveCondition E, formalism::IsFluentOrDerivedTag P>
class InverseAtomSelectorNode_TX;
template<formalism::HasConjunctiveCondition E, formalism::IsFluentOrDerivedTag P>
class InverseAtomSelectorNode_FX;
template<formalism::HasConjunctiveCondition E, formalism::IsFluentOrDerivedTag P>
class InverseAtomSelectorNode_T;
template<formalism::HasConjunctiveCondition E, formalism::IsFluentOrDerivedTag P>
class InverseAtomSelectorNode_F;
template<formalism::HasConjunctiveCondition E>
class InverseNumericConstraintSelectorNode_TX;
template<formalism::HasConjunctiveCondition E>
class InverseNumericConstraintSelectorNode_T;
template<formalism::HasConjunctiveCondition E>
class InverseElementGeneratorNode_Perfect;
template<formalism::HasConjunctiveCondition E>
class InverseElementGeneratorNode_Imperfect;

template<formalism::HasConjunctiveCondition E>
class INode;
template<formalism::HasConjunctiveCondition E, formalism::IsFluentOrDerivedTag P>
class AtomSelectorNode_TFX;
template<formalism::HasConjunctiveCondition E, formalism::IsFluentOrDerivedTag P>
class AtomSelectorNode_TF;
template<formalism::HasConjunctiveCondition E, formalism::IsFluentOrDerivedTag P>
class AtomSelectorNode_TX;
template<formalism::HasConjunctiveCondition E, formalism::IsFluentOrDerivedTag P>
class AtomSelectorNode_FX;
template<formalism::HasConjunctiveCondition E, formalism::IsFluentOrDerivedTag P>
class AtomSelectorNode_T;
template<formalism::HasConjunctiveCondition E, formalism::IsFluentOrDerivedTag P>
class AtomSelectorNode_F;
template<formalism::HasConjunctiveCondition E>
class NumericConstraintSelectorNode_TX;
template<formalism::HasConjunctiveCondition E>
class NumericConstraintSelectorNode_T;
template<formalism::HasConjunctiveCondition E>
class ElementGeneratorNode_Perfect;
template<formalism::HasConjunctiveCondition E>
class ElementGeneratorNode_Imperfect;

template<formalism::HasConjunctiveCondition E>
class INodeScoreFunction;

template<formalism::HasConjunctiveCondition E>
class INodeSplitter;

struct Options;
struct Statistics;

template<formalism::HasConjunctiveCondition E>
class MatchTreeImpl;
template<formalism::HasConjunctiveCondition E>
using MatchTree = std::unique_ptr<MatchTreeImpl<E>>;
template<formalism::HasConjunctiveCondition E>
using MatchTreeList = std::vector<MatchTree<E>>;

/**
 * Aliases
 */

template<formalism::HasConjunctiveCondition E>
using PlaceholderNode = std::unique_ptr<PlaceholderNodeImpl<E>>;
template<formalism::HasConjunctiveCondition E>
using PlaceholderNodeList = std::vector<PlaceholderNode<E>>;

template<formalism::HasConjunctiveCondition E>
using InverseNode = std::unique_ptr<IInverseNode<E>>;
template<formalism::HasConjunctiveCondition E>
using InverseNodeList = std::vector<InverseNode<E>>;

template<formalism::HasConjunctiveCondition E>
using Node = std::unique_ptr<INode<E>>;

template<formalism::HasConjunctiveCondition E>
using NodeScoreFunction = std::unique_ptr<INodeScoreFunction<E>>;

template<formalism::HasConjunctiveCondition E>
using NodeSplitter = std::unique_ptr<INodeSplitter<E>>;
}

#endif
