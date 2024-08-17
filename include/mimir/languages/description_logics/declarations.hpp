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

#ifndef MIMIR_LANGUAGES_DESCRIPTION_LOGICS_DECLARATIONS_HPP_
#define MIMIR_LANGUAGES_DESCRIPTION_LOGICS_DECLARATIONS_HPP_

#include "mimir/formalism/predicate_category.hpp"
#include "mimir/languages/description_logics/constructor_category.hpp"

#include <variant>
#include <vector>

namespace mimir::dl
{
template<typename T>
class ConstructorRepository;

class EvaluationContext;

/**
 * Constructors
 */

/* Constructor */
template<IsConceptOrRole D>
class ConstructorImpl;
template<IsConceptOrRole D>
using Constructor = const ConstructorImpl<D>*;
template<IsConceptOrRole D>
using ConstructorList = std::vector<Constructor<D>>;

/* Concrete concepts */
template<PredicateCategory P>
class ConceptPredicateStateImpl;
template<PredicateCategory P>
using ConceptPredicateState = const ConceptPredicateStateImpl<P>*;
template<PredicateCategory P>
class ConceptPredicateGoalImpl;
template<PredicateCategory P>
using ConceptPredicateGoal = const ConceptPredicateGoalImpl<P>*;
class ConceptAndImpl;
using ConceptAnd = const ConceptAndImpl*;

/* Concrete roles */
template<PredicateCategory P>
class RolePredicateStateImpl;
template<PredicateCategory P>
using RolePredicateState = const RolePredicateStateImpl<P>*;
template<PredicateCategory P>
class RolePredicateGoalImpl;
template<PredicateCategory P>
using RolePredicateGoal = const RolePredicateGoalImpl<P>*;
class RoleAndImpl;
using RoleAnd = const RoleAndImpl*;

namespace grammar
{
/**
 * Constructors
 */

/* Constructor */
template<dl::IsConceptOrRole D>
class ConstructorImpl;
template<dl::IsConceptOrRole D>
using Constructor = const ConstructorImpl<D>*;

/* DerivationRule */
template<dl::IsConceptOrRole D>
class DerivationRuleImpl;
template<dl::IsConceptOrRole D>
using DerivationRule = const DerivationRuleImpl<D>*;
template<IsConceptOrRole D>
using DerivationRuleList = std::vector<DerivationRule<D>>;

/* NonTerminal */
template<dl::IsConceptOrRole D>
class NonTerminalImpl;
template<dl::IsConceptOrRole D>
using NonTerminal = const NonTerminalImpl<D>*;

/* ConstructorOrNonTerminalChoice */
template<IsConceptOrRole D>
using ConstructorOrNonTerminalChoice = std::variant<Constructor<D>, NonTerminal<D>>;

/* Choice */
template<dl::IsConceptOrRole D>
class ChoiceImpl;
template<dl::IsConceptOrRole D>
using Choice = const ChoiceImpl<D>*;
template<dl::IsConceptOrRole D>
using ChoiceList = std::vector<Choice<D>>;

/* Concrete concepts */
template<PredicateCategory P>
class ConceptPredicateStateImpl;
template<PredicateCategory P>
using ConceptPredicateState = const ConceptPredicateStateImpl<P>*;
template<PredicateCategory P>
class ConceptPredicateGoalImpl;
template<PredicateCategory P>
using ConceptPredicateGoal = const ConceptPredicateGoalImpl<P>*;
class ConceptAndImpl;
using ConceptAnd = const ConceptAndImpl*;

/* Concrete roles */
template<PredicateCategory P>
class RolePredicateStateImpl;
template<PredicateCategory P>
using RolePredicateState = const RolePredicateStateImpl<P>*;
template<PredicateCategory P>
class RolePredicateGoalImpl;
template<PredicateCategory P>
using RolePredicateGoal = const RolePredicateGoalImpl<P>*;
class RoleAndImpl;
using RoleAnd = const RoleAndImpl*;

/**
 * Visitors
 */

template<dl::IsConceptOrRole D>
class Visitor;

}
}

#endif