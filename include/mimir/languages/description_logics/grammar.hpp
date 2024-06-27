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
#include "mimir/languages/description_logics/constructors.hpp"
#include "mimir/languages/description_logics/grammar_interface.hpp"

#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

namespace mimir::dl
{

template<IsDLConstructor D>
class DerivationRule;

/**
 * TerminalSymbols
 */

class TerminalSymbolConceptPredicate : public TerminalSymbol<Concept>
{
public:
};

class TerminalSymbolConceptBot : public TerminalSymbol<Concept>
{
public:
};

class TerminalSymbolConceptTop : public TerminalSymbol<Concept>
{
};

class TerminalSymbolRolePredicate : public TerminalSymbol<Role>
{
};

class TerminalSymbolRoleBot : public TerminalSymbol<Role>
{
};

class TerminalSymbolRoleTop : public TerminalSymbol<Role>
{
};

/**
 * NonterminalSymbols
 */

class NonTerminalSymbolConceptPredicate : public NonTerminalSymbol<Concept>
{
};

class NonTerminalSymbolConceptAll : public NonTerminalSymbol<Concept>
{
};

class NonTerminalSymbolConceptEqual : public NonTerminalSymbol<Concept>
{
};

class NonTerminalSymbolRolePredicate : public NonTerminalSymbol<Role>
{
};

/**
 * SequenceRules
 */

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
/// <concept_primitive> ::= "predicate\1" | "predicate_g\1"
/// <role_primitive> ::= "predicate\2" | "predicate_g\2"
/// <concept_all> ::= <role> <concept>
/// <concept_equal> ::= "predicate_g\2" "predicate\2"
/// <concept> ::= <concept_primitive> | "bot" | "top" | <concept_all> | <concept_equal>
/// <role> ::= <role_primitive> | "bot" | "top"

class Grammar
{
    // Store all remaining rules of concrete concepts and roles.
    std::unique_ptr<DerivationRule<ConceptPredicate<Static>>> m_concept_static_primitive;
    std::unique_ptr<DerivationRule<ConceptPredicate<Fluent>>> m_concept_fluent_primitive;
    std::unique_ptr<DerivationRule<ConceptPredicate<Derived>>> m_concept_derived_primitive;
    std::unique_ptr<DerivationRule<ConceptAll>> m_concept_all;
    // TODO: Add remaining concrete concept and rules at a later time

    // Root rules
    std::unique_ptr<DerivationRule<Concept>> m_concept_derivation_rule;
    std::unique_ptr<DerivationRule<Role>> m_role_derivation_rule;
};

}

#endif
