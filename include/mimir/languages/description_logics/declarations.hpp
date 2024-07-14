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
#include "mimir/languages/description_logics/constructor_ids.hpp"

#include <variant>
#include <vector>

namespace mimir::dl
{
template<typename T>
class ConstructorRepository;

struct EvaluationContext;

/**
 * Constructors
 */

template<IsConceptOrRole D>
class Constructor;
using ConceptConstructorList = std::vector<std::reference_wrapper<const Constructor<Concept>>>;
using RoleConstructorList = std::vector<std::reference_wrapper<const Constructor<Role>>>;

template<PredicateCategory P>
class ConceptPredicateState;
template<PredicateCategory P>
class ConceptPredicateGoal;
class ConceptAnd;

template<PredicateCategory P>
class RolePredicateState;
template<PredicateCategory P>
class RolePredicateGoal;
class RoleAnd;

namespace grammar
{
/**
 * Constructors
 */

/* Constructor */
template<dl::IsConceptOrRole D>
class Constructor;

/* DerivationRule */
template<dl::IsConceptOrRole D>
class DerivationRule;
template<IsConceptOrRole D>
using DerivationRuleList = std::vector<std::reference_wrapper<const DerivationRule<D>>>;
using ConceptDerivationRule = DerivationRule<Concept>;
using ConceptDerivationRuleList = DerivationRuleList<Concept>;
using RoleDerivationRule = DerivationRule<Role>;
using RoleDerivationRuleList = DerivationRuleList<Role>;

/* NonTerminal */
template<dl::IsConceptOrRole D>
class NonTerminal;
using ConceptNonTerminal = NonTerminal<Concept>;
using RoleNonTerminal = NonTerminal<Role>;

/* ConstructorOrNonTerminalChoice */
template<IsConceptOrRole D>
using ConstructorOrNonTerminalChoice = std::variant<std::reference_wrapper<const Constructor<D>>, std::reference_wrapper<const NonTerminal<D>>>;

/* Choice */
template<dl::IsConceptOrRole D>
class Choice;
template<dl::IsConceptOrRole D>
using ChoiceList = std::vector<std::reference_wrapper<const Choice<D>>>;
using ConceptChoice = Choice<Concept>;
using ConceptChoiceList = ChoiceList<Concept>;
using RoleChoice = Choice<Role>;
using RoleChoiceList = ChoiceList<Role>;

/**
 * Visitors
 */

class ConceptVisitor;
class RoleVisitor;

}
}

#endif