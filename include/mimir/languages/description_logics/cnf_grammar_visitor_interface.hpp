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

#ifndef MIMIR_LANGUAGES_DESCRIPTION_LOGICS_CNF_GRAMMAR_VISITOR_INTERFACE_HPP_
#define MIMIR_LANGUAGES_DESCRIPTION_LOGICS_CNF_GRAMMAR_VISITOR_INTERFACE_HPP_

#include "mimir/languages/description_logics/declarations.hpp"

namespace mimir::dl::cnf_grammar
{

////////////////////////////
/// Interface
////////////////////////////

/**
 * Constructors
 */

template<ConceptOrRole D>
class ConstructorVisitor
{
};

/**
 * Concept
 */

template<>
class ConstructorVisitor<Concept>
{
public:
    virtual ~ConstructorVisitor() = default;

    virtual void visit(ConceptBot constructor) = 0;
    virtual void visit(ConceptTop constructor) = 0;
    virtual void visit(ConceptAtomicState<Static> constructor) = 0;
    virtual void visit(ConceptAtomicState<Fluent> constructor) = 0;
    virtual void visit(ConceptAtomicState<Derived> constructor) = 0;
    virtual void visit(ConceptAtomicGoal<Static> constructor) = 0;
    virtual void visit(ConceptAtomicGoal<Fluent> constructor) = 0;
    virtual void visit(ConceptAtomicGoal<Derived> constructor) = 0;
    virtual void visit(ConceptNominal constructor) = 0;
    virtual void visit(ConceptIntersection constructor) = 0;
    virtual void visit(ConceptUnion constructor) = 0;
    virtual void visit(ConceptNegation constructor) = 0;
    virtual void visit(ConceptValueRestriction constructor) = 0;
    virtual void visit(ConceptExistentialQuantification constructor) = 0;
    virtual void visit(ConceptRoleValueMapContainment constructor) = 0;
    virtual void visit(ConceptRoleValueMapEquality constructor) = 0;
};

/**
 * Role
 */

template<>
class ConstructorVisitor<Role>
{
public:
    virtual ~ConstructorVisitor() = default;

    virtual void visit(RoleUniversal constructor) = 0;
    virtual void visit(RoleAtomicState<Static> constructor) = 0;
    virtual void visit(RoleAtomicState<Fluent> constructor) = 0;
    virtual void visit(RoleAtomicState<Derived> constructor) = 0;
    virtual void visit(RoleAtomicGoal<Static> constructor) = 0;
    virtual void visit(RoleAtomicGoal<Fluent> constructor) = 0;
    virtual void visit(RoleAtomicGoal<Derived> constructor) = 0;
    virtual void visit(RoleIntersection constructor) = 0;
    virtual void visit(RoleUnion constructor) = 0;
    virtual void visit(RoleComplement constructor) = 0;
    virtual void visit(RoleInverse constructor) = 0;
    virtual void visit(RoleComposition constructor) = 0;
    virtual void visit(RoleTransitiveClosure constructor) = 0;
    virtual void visit(RoleReflexiveTransitiveClosure constructor) = 0;
    virtual void visit(RoleRestriction constructor) = 0;
    virtual void visit(RoleIdentity constructor) = 0;
};

/**
 * NonTerminal
 */

template<ConceptOrRole D>
class NonTerminalVisitor
{
public:
    virtual ~NonTerminalVisitor() = default;

    virtual void visit(NonTerminal<D> constructor) = 0;
};

/**
 * DerivationRule
 */

template<ConceptOrRole D>
class DerivationRuleVisitor
{
public:
    virtual ~DerivationRuleVisitor() = default;

    virtual void visit(DerivationRule<D> rule) = 0;
};

/**
 * SubstitutionRule
 */

template<ConceptOrRole D>
class SubstitutionRuleVisitor
{
public:
    virtual ~SubstitutionRuleVisitor() = default;

    virtual void visit(SubstitutionRule<D> rule) = 0;
};

/**
 * Grammar
 */

class GrammarVisitor
{
public:
    virtual ~GrammarVisitor() = default;

    virtual void visit(const Grammar& grammar) = 0;
};

}

#endif
