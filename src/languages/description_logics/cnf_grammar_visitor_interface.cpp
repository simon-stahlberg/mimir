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

#include "mimir/languages/description_logics/cnf_grammar_visitor_interface.hpp"

#include "mimir/languages/description_logics/cnf_grammar_constructors.hpp"

namespace mimir::dl::cnf_grammar
{

CopyConstructorVisitor<Concept>::CopyConstructorVisitor(ConstructorRepositories& repositories) : m_repositories(repositories) {}

void CopyConstructorVisitor<Concept>::visit(ConceptBot constructor) {}
void CopyConstructorVisitor<Concept>::visit(ConceptTop constructor) {}
void CopyConstructorVisitor<Concept>::visit(ConceptAtomicState<Static> constructor) {}
void CopyConstructorVisitor<Concept>::visit(ConceptAtomicState<Fluent> constructor) {}
void CopyConstructorVisitor<Concept>::visit(ConceptAtomicState<Derived> constructor) {}
void CopyConstructorVisitor<Concept>::visit(ConceptAtomicGoal<Static> constructor) {}
void CopyConstructorVisitor<Concept>::visit(ConceptAtomicGoal<Fluent> constructor) {}
void CopyConstructorVisitor<Concept>::visit(ConceptAtomicGoal<Derived> constructor) {}
void CopyConstructorVisitor<Concept>::visit(ConceptNominal constructor) {}
void CopyConstructorVisitor<Concept>::visit(ConceptIntersection constructor) {}
void CopyConstructorVisitor<Concept>::visit(ConceptUnion constructor) {}
void CopyConstructorVisitor<Concept>::visit(ConceptNegation constructor) {}
void CopyConstructorVisitor<Concept>::visit(ConceptValueRestriction constructor) {}
void CopyConstructorVisitor<Concept>::visit(ConceptExistentialQuantification constructor) {}
void CopyConstructorVisitor<Concept>::visit(ConceptRoleValueMapContainment constructor) {}
void CopyConstructorVisitor<Concept>::visit(ConceptRoleValueMapEquality constructor) {}

/**
 * Role
 */

CopyConstructorVisitor<Role>::CopyConstructorVisitor(ConstructorRepositories& repositories) : m_repositories(repositories) {}

void CopyConstructorVisitor<Role>::visit(RoleUniversal constructor) {}
void CopyConstructorVisitor<Role>::visit(RoleAtomicState<Static> constructor) {}
void CopyConstructorVisitor<Role>::visit(RoleAtomicState<Fluent> constructor) {}
void CopyConstructorVisitor<Role>::visit(RoleAtomicState<Derived> constructor) {}
void CopyConstructorVisitor<Role>::visit(RoleAtomicGoal<Static> constructor) {}
void CopyConstructorVisitor<Role>::visit(RoleAtomicGoal<Fluent> constructor) {}
void CopyConstructorVisitor<Role>::visit(RoleAtomicGoal<Derived> constructor) {}
void CopyConstructorVisitor<Role>::visit(RoleIntersection constructor) {}
void CopyConstructorVisitor<Role>::visit(RoleUnion constructor) {}
void CopyConstructorVisitor<Role>::visit(RoleComplement constructor) {}
void CopyConstructorVisitor<Role>::visit(RoleInverse constructor) {}
void CopyConstructorVisitor<Role>::visit(RoleComposition constructor) {}
void CopyConstructorVisitor<Role>::visit(RoleTransitiveClosure constructor) {}
void CopyConstructorVisitor<Role>::visit(RoleReflexiveTransitiveClosure constructor) {}
void CopyConstructorVisitor<Role>::visit(RoleRestriction constructor) {}
void CopyConstructorVisitor<Role>::visit(RoleIdentity constructor) {}

/**
 * NonTerminal
 */

template<ConceptOrRole D>
CopyNonTerminalVisitor<D>::CopyNonTerminalVisitor(ConstructorRepositories& repositories) : m_repositories(repositories)
{
}

template<ConceptOrRole D>
void CopyNonTerminalVisitor<D>::visit(NonTerminal<D> constructor)
{
}

template class CopyNonTerminalVisitor<Concept>;
template class CopyNonTerminalVisitor<Role>;

/**
 * DerivationRule
 */

template<ConceptOrRole D>
CopyDerivationRuleVisitor<D>::CopyDerivationRuleVisitor(ConstructorRepositories& repositories) : m_repositories(repositories)
{
}

template<ConceptOrRole D>
void CopyDerivationRuleVisitor<D>::visit(DerivationRule<D> rule)
{
}

template class CopyDerivationRuleVisitor<Concept>;
template class CopyDerivationRuleVisitor<Role>;

/**
 * SubstitutionRule
 */

template<ConceptOrRole D>
CopySubstitutionRuleVisitor<D>::CopySubstitutionRuleVisitor(ConstructorRepositories& repositories) : m_repositories(repositories)
{
}

template<ConceptOrRole D>
void CopySubstitutionRuleVisitor<D>::visit(SubstitutionRule<D> rule)
{
}

template class CopySubstitutionRuleVisitor<Concept>;
template class CopySubstitutionRuleVisitor<Role>;

/**
 * Grammar
 */

CopyGrammarVisitor::CopyGrammarVisitor(ConstructorRepositories& repositories) : m_repositories(repositories) {}

void CopyGrammarVisitor::visit(const Grammar& grammar) {}

}
