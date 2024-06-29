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

#ifndef MIMIR_LANGUAGES_DESCRIPTION_LOGICS_GRAMMAR_HPP_
#define MIMIR_LANGUAGES_DESCRIPTION_LOGICS_GRAMMAR_HPP_

#include "mimir/formalism/predicate.hpp"
#include "mimir/languages/description_logics/grammar_interface.hpp"

#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

namespace mimir::dl
{
/**
 * Forward declaration
 */

class Concept;
class Role;

/**
 * TerminalSymbols
 */

class TerminalSymbolConceptPredicate : public TerminalSymbol
{
public:
    bool test_match(const Concept& constructor) const override;
};

class TerminalSymbolConceptAnd : public TerminalSymbol
{
public:
    bool test_match(const Concept& constructor) const override;
};

/**
 * NonterminalSymbols
 */

class NonTerminalSymbolConceptPredicate : public NonTerminalSymbol
{
    bool test_match(const Concept& constructor) const override;
};

class NonTerminalSymbolConceptAnd : public NonTerminalSymbol
{
    bool test_match(const Concept& constructor) const override;
};

/**
 * ChoiceRules
 */

/**
 * DerivationRules
 */

/**
 * Grammar
 */

/// Example BNF grammar DL fragment with restriction on role-value map (equal)
/// <concept_predicate1_state> ::= @concept_predicate_state "predicate1"
/// <concept_predicate1_goal> ::= @concept_predicate_goal "predicate1"
/// <concept_all> ::= "@concept_all" <role> <concept>
/// <concept_equal> ::= "@concept_equal" <concept_predicate1_state> <concept_predicate1_goal>
/// <concept_bot> ::= "@concept_bot"
/// <concept_top> ::= "@concept_top"
/// <concept> ::= <concept_predicate1_state> | <concept_predicate1_goal> | <concept_bot> | <concept_top> | <concept_all> | <concept_equal>
/// <role_predicate2_state> ::= @role_predicate_state "predicate2"
/// <role_predicate2_goal> ::= @role_predicate_goal "predicate2"
/// <role_bot> ::= "@role_bot"
/// <role_top> ::= "@role_top"
/// <role> ::= <role_predicate2_state> | <role_predicate2_goal> | <role_bot> | <role_top>

class Grammar
{
    std::vector<const DerivationRule*> m_rules;
};

}

#endif
