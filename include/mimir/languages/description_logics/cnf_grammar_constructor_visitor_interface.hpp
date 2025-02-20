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

#ifndef MIMIR_LANGUAGES_DESCRIPTION_LOGICS_CNF_GRAMMAR_CONSTRUCTOR_VISITOR_INTERFACE_HPP_
#define MIMIR_LANGUAGES_DESCRIPTION_LOGICS_CNF_GRAMMAR_CONSTRUCTOR_VISITOR_INTERFACE_HPP_

#include "mimir/languages/description_logics/declarations.hpp"

namespace mimir::dl::cnf_grammar
{
/**
 * Concept
 */

template<ConceptOrRole D>
class ConstructorVisitor
{
};

template<>
class ConstructorVisitor<Concept>
{
protected:
    NonTerminalVisitor<Concept>* m_concept_nonterminal_visitor;
    NonTerminalVisitor<Role>* m_role_nonterminal_visitor;

public:
    ConstructorVisitor() = default;

    virtual ~ConstructorVisitor() = default;

    virtual void initialize(NonTerminalVisitor<Concept>& concept_nonterminal_visitor, NonTerminalVisitor<Role>& role_nonterminal_visitor);

    virtual void visit(ConceptBot constructor);
    virtual void visit(ConceptTop constructor);
    virtual void visit(ConceptAtomicState<Static> constructor);
    virtual void visit(ConceptAtomicState<Fluent> constructor);
    virtual void visit(ConceptAtomicState<Derived> constructor);
    virtual void visit(ConceptAtomicGoal<Static> constructor);
    virtual void visit(ConceptAtomicGoal<Fluent> constructor);
    virtual void visit(ConceptAtomicGoal<Derived> constructor);
    virtual void visit(ConceptIntersection constructor);
    virtual void visit(ConceptUnion constructor);
    virtual void visit(ConceptNegation constructor);
    virtual void visit(ConceptValueRestriction constructor);
    virtual void visit(ConceptExistentialQuantification constructor);
    virtual void visit(ConceptRoleValueMapContainment constructor);
    virtual void visit(ConceptRoleValueMapEquality constructor);
    virtual void visit(ConceptNominal constructor);
};

/**
 * Role
 */

template<>
class ConstructorVisitor<Role>
{
protected:
    NonTerminalVisitor<Concept>* m_concept_nonterminal_visitor;
    NonTerminalVisitor<Role>* m_role_nonterminal_visitor;

public:
    ConstructorVisitor() = default;

    virtual ~ConstructorVisitor() = default;

    virtual void initialize(NonTerminalVisitor<Concept>& concept_nonterminal_visitor, NonTerminalVisitor<Role>& role_nonterminal_visitor);

    virtual void visit(RoleUniversal constructor);
    virtual void visit(RoleAtomicState<Static> constructor);
    virtual void visit(RoleAtomicState<Fluent> constructor);
    virtual void visit(RoleAtomicState<Derived> constructor);
    virtual void visit(RoleAtomicGoal<Static> constructor);
    virtual void visit(RoleAtomicGoal<Fluent> constructor);
    virtual void visit(RoleAtomicGoal<Derived> constructor);
    virtual void visit(RoleIntersection constructor);
    virtual void visit(RoleUnion constructor);
    virtual void visit(RoleComplement constructor);
    virtual void visit(RoleInverse constructor);
    virtual void visit(RoleComposition constructor);
    virtual void visit(RoleTransitiveClosure constructor);
    virtual void visit(RoleReflexiveTransitiveClosure constructor);
    virtual void visit(RoleRestriction constructor);
    virtual void visit(RoleIdentity constructor);
};

/**
 * NonTerminal
 */

template<ConceptOrRole D>
class NonTerminalVisitor
{
public:
    virtual ~NonTerminalVisitor() = default;

    virtual void visit(NonTerminal<D> constructor);
};
}

#endif
