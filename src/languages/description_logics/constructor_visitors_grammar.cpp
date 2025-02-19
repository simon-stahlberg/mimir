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

#include "mimir/languages/description_logics/constructor_visitors_grammar.hpp"

#include "mimir/languages/description_logics/constructors.hpp"
#include "mimir/languages/description_logics/grammar_constructors.hpp"

namespace mimir::dl
{

/**
 * ConceptBotGrammarVisitor
 */

ConceptBotGrammarVisitor::ConceptBotGrammarVisitor(grammar::ConceptBot grammar_constructor, const grammar::Grammar& grammar) :
    m_grammar_constructor(grammar_constructor),
    m_grammar(grammar)
{
}

void ConceptBotGrammarVisitor::visit(ConceptBot constructor) { m_result = true; }

/**
 * ConceptTopGrammarVisitor
 */

ConceptTopGrammarVisitor::ConceptTopGrammarVisitor(grammar::ConceptTop grammar_constructor, const grammar::Grammar& grammar) :
    m_grammar_constructor(grammar_constructor),
    m_grammar(grammar)
{
}

void ConceptTopGrammarVisitor::visit(ConceptTop constructor) { m_result = true; }

/**
 * ConceptAtomicStateGrammarVisitor
 */

template<StaticOrFluentOrDerived P>
ConceptAtomicStateGrammarVisitor<P>::ConceptAtomicStateGrammarVisitor(grammar::ConceptAtomicState<P> grammar_constructor, const grammar::Grammar& grammar) :
    m_grammar_constructor(grammar_constructor),
    m_grammar(grammar)
{
}

template<StaticOrFluentOrDerived P>
void ConceptAtomicStateGrammarVisitor<P>::visit(ConceptAtomicState<P> constructor)
{
    m_result = constructor->get_predicate() == m_grammar_constructor->get_predicate();
}

template class ConceptAtomicStateGrammarVisitor<Static>;
template class ConceptAtomicStateGrammarVisitor<Fluent>;
template class ConceptAtomicStateGrammarVisitor<Derived>;

/**
 * ConceptAtomicGoalGrammarVisitor
 */

template<StaticOrFluentOrDerived P>
ConceptAtomicGoalGrammarVisitor<P>::ConceptAtomicGoalGrammarVisitor(grammar::ConceptAtomicGoal<P> grammar_constructor, const grammar::Grammar& grammar) :
    m_grammar_constructor(grammar_constructor),
    m_grammar(grammar)
{
}

template<StaticOrFluentOrDerived P>
void ConceptAtomicGoalGrammarVisitor<P>::visit(ConceptAtomicGoal<P> constructor)
{
    m_result = constructor->get_predicate() == m_grammar_constructor->get_predicate();
}

template class ConceptAtomicGoalGrammarVisitor<Static>;
template class ConceptAtomicGoalGrammarVisitor<Fluent>;
template class ConceptAtomicGoalGrammarVisitor<Derived>;

/**
 * ConceptIntersectionGrammarVisitor
 */

ConceptIntersectionGrammarVisitor::ConceptIntersectionGrammarVisitor(grammar::ConceptIntersection grammar_constructor, const grammar::Grammar& grammar) :
    m_grammar_constructor(grammar_constructor),
    m_grammar(grammar)
{
}

void ConceptIntersectionGrammarVisitor::visit(ConceptIntersection constructor)
{
    m_result = m_grammar_constructor->get_concept_or_non_terminal_left()->test_match(constructor->get_concept_left(), m_grammar)  //
               && m_grammar_constructor->get_concept_or_non_terminal_right()->test_match(constructor->get_concept_right(), m_grammar);
}

/**
 * ConceptUnionGrammarVisitor
 */

ConceptUnionGrammarVisitor::ConceptUnionGrammarVisitor(grammar::ConceptUnion grammar_constructor, const grammar::Grammar& grammar) :
    m_grammar_constructor(grammar_constructor),
    m_grammar(grammar)
{
}

void ConceptUnionGrammarVisitor::visit(ConceptUnion constructor)
{
    m_result = m_grammar_constructor->get_concept_or_non_terminal_left()->test_match(constructor->get_concept_left(), m_grammar)  //
               && m_grammar_constructor->get_concept_or_non_terminal_right()->test_match(constructor->get_concept_right(), m_grammar);
}

/**
 * ConceptNegationGrammarVisitor
 */

ConceptNegationGrammarVisitor::ConceptNegationGrammarVisitor(grammar::ConceptNegation grammar_constructor, const grammar::Grammar& grammar) :
    m_grammar_constructor(grammar_constructor),
    m_grammar(grammar)
{
}

void ConceptNegationGrammarVisitor::visit(ConceptNegation constructor)
{
    m_result = m_grammar_constructor->get_concept_or_non_terminal()->test_match(constructor->get_concept(), m_grammar);
}

/**
 * ConceptValueRestrictionGrammarVisitor
 */

ConceptValueRestrictionGrammarVisitor::ConceptValueRestrictionGrammarVisitor(grammar::ConceptValueRestriction grammar_constructor,
                                                                             const grammar::Grammar& grammar) :
    m_grammar_constructor(grammar_constructor),
    m_grammar(grammar)
{
}

void ConceptValueRestrictionGrammarVisitor::visit(ConceptValueRestriction constructor)
{
    m_result = m_grammar_constructor->get_role_or_non_terminal()->test_match(constructor->get_role(), m_grammar)  //
               && m_grammar_constructor->get_concept_or_non_terminal()->test_match(constructor->get_concept(), m_grammar);
}

/**
 * ConceptExistentialQuantificationGrammarVisitor
 */

ConceptExistentialQuantificationGrammarVisitor::ConceptExistentialQuantificationGrammarVisitor(grammar::ConceptExistentialQuantification grammar_constructor,
                                                                                               const grammar::Grammar& grammar) :
    m_grammar_constructor(grammar_constructor),
    m_grammar(grammar)
{
}

void ConceptExistentialQuantificationGrammarVisitor::visit(ConceptExistentialQuantification constructor)
{
    m_result = m_grammar_constructor->get_role_or_non_terminal()->test_match(constructor->get_role(), m_grammar)  //
               && m_grammar_constructor->get_concept_or_non_terminal()->test_match(constructor->get_concept(), m_grammar);
}

/**
 * ConceptRoleValueMapContainmentGrammarVisitor
 */

ConceptRoleValueMapContainmentGrammarVisitor::ConceptRoleValueMapContainmentGrammarVisitor(grammar::ConceptRoleValueMapContainment grammar_constructor,
                                                                                           const grammar::Grammar& grammar) :
    m_grammar_constructor(grammar_constructor),
    m_grammar(grammar)
{
}

void ConceptRoleValueMapContainmentGrammarVisitor::visit(ConceptRoleValueMapContainment constructor)
{
    m_result = m_grammar_constructor->get_role_or_non_terminal_left()->test_match(constructor->get_role_left(), m_grammar)  //
               && m_grammar_constructor->get_role_or_non_terminal_right()->test_match(constructor->get_role_right(), m_grammar);
}

/**
 * ConceptRoleValueMapEqualityGrammarVisitor
 */

ConceptRoleValueMapEqualityGrammarVisitor::ConceptRoleValueMapEqualityGrammarVisitor(grammar::ConceptRoleValueMapEquality grammar_constructor,
                                                                                     const grammar::Grammar& grammar) :
    m_grammar_constructor(grammar_constructor),
    m_grammar(grammar)
{
}

void ConceptRoleValueMapEqualityGrammarVisitor::visit(ConceptRoleValueMapEquality constructor)
{
    m_result = m_grammar_constructor->get_role_or_non_terminal_left()->test_match(constructor->get_role_left(), m_grammar)  //
               && m_grammar_constructor->get_role_or_non_terminal_right()->test_match(constructor->get_role_right(), m_grammar);
}

/**
 * ConceptNominalGrammarVisitor
 */

ConceptNominalGrammarVisitor::ConceptNominalGrammarVisitor(grammar::ConceptNominal grammar_constructor, const grammar::Grammar& grammar) :
    m_grammar_constructor(grammar_constructor),
    m_grammar(grammar)
{
}

void ConceptNominalGrammarVisitor::visit(ConceptNominal constructor) { m_result = m_grammar_constructor->get_object() == constructor->get_object(); }

/**
 * RoleUniversalGrammarVisitor
 */

RoleUniversalGrammarVisitor::RoleUniversalGrammarVisitor(grammar::RoleUniversal grammar_constructor, const grammar::Grammar& grammar) :
    m_grammar_constructor(grammar_constructor),
    m_grammar(grammar)
{
}

void RoleUniversalGrammarVisitor::visit(RoleUniversal constructor) { m_result = true; }

/**
 * RoleAtomicStateGrammarVisitor
 */

template<StaticOrFluentOrDerived P>
RoleAtomicStateGrammarVisitor<P>::RoleAtomicStateGrammarVisitor(grammar::RoleAtomicState<P> grammar_constructor, const grammar::Grammar& grammar) :
    m_grammar_constructor(grammar_constructor),
    m_grammar(grammar)
{
}

template<StaticOrFluentOrDerived P>
void RoleAtomicStateGrammarVisitor<P>::visit(RoleAtomicState<P> constructor)
{
    m_result = constructor->get_predicate() == m_grammar_constructor->get_predicate();
}

template class RoleAtomicStateGrammarVisitor<Static>;
template class RoleAtomicStateGrammarVisitor<Fluent>;
template class RoleAtomicStateGrammarVisitor<Derived>;

/**
 * RoleAtomicGoalGrammarVisitor
 */

template<StaticOrFluentOrDerived P>
RoleAtomicGoalGrammarVisitor<P>::RoleAtomicGoalGrammarVisitor(grammar::RoleAtomicGoal<P> grammar_constructor, const grammar::Grammar& grammar) :
    m_grammar_constructor(grammar_constructor),
    m_grammar(grammar)
{
}

template<StaticOrFluentOrDerived P>
void RoleAtomicGoalGrammarVisitor<P>::visit(RoleAtomicGoal<P> constructor)
{
    m_result = (constructor->get_predicate() == m_grammar_constructor->get_predicate());
}

template class RoleAtomicGoalGrammarVisitor<Static>;
template class RoleAtomicGoalGrammarVisitor<Fluent>;
template class RoleAtomicGoalGrammarVisitor<Derived>;

/**
 * RoleIntersectionGrammarVisitor
 */

RoleIntersectionGrammarVisitor::RoleIntersectionGrammarVisitor(grammar::RoleIntersection grammar_constructor, const grammar::Grammar& grammar) :
    m_grammar_constructor(grammar_constructor),
    m_grammar(grammar)
{
}

void RoleIntersectionGrammarVisitor::visit(RoleIntersection constructor)
{
    m_result = m_grammar_constructor->get_role_or_non_terminal_left()->test_match(constructor->get_role_left(), m_grammar)
               && m_grammar_constructor->get_role_or_non_terminal_right()->test_match(constructor->get_role_right(), m_grammar);
}

/**
 * RoleUnionGrammarVisitor
 */

RoleUnionGrammarVisitor::RoleUnionGrammarVisitor(grammar::RoleUnion grammar_constructor, const grammar::Grammar& grammar) :
    m_grammar_constructor(grammar_constructor),
    m_grammar(grammar)
{
}

void RoleUnionGrammarVisitor::visit(RoleUnion constructor)
{
    m_result = m_grammar_constructor->get_role_or_non_terminal_left()->test_match(constructor->get_role_left(), m_grammar)
               && m_grammar_constructor->get_role_or_non_terminal_right()->test_match(constructor->get_role_right(), m_grammar);
}

/**
 * RoleComplementGrammarVisitor
 */

RoleComplementGrammarVisitor::RoleComplementGrammarVisitor(grammar::RoleComplement grammar_constructor, const grammar::Grammar& grammar) :
    m_grammar_constructor(grammar_constructor),
    m_grammar(grammar)
{
}

void RoleComplementGrammarVisitor::visit(RoleComplement constructor)
{
    m_result = m_grammar_constructor->get_role_or_non_terminal()->test_match(constructor->get_role(), m_grammar);
}

/**
 * RoleInverseGrammarVisitor
 */

RoleInverseGrammarVisitor::RoleInverseGrammarVisitor(grammar::RoleInverse grammar_constructor, const grammar::Grammar& grammar) :
    m_grammar_constructor(grammar_constructor),
    m_grammar(grammar)
{
}

void RoleInverseGrammarVisitor::visit(RoleInverse constructor)
{
    m_result = m_grammar_constructor->get_role_or_non_terminal()->test_match(constructor->get_role(), m_grammar);
}

/**
 * RoleCompositionGrammarVisitor
 */

RoleCompositionGrammarVisitor::RoleCompositionGrammarVisitor(grammar::RoleComposition grammar_constructor, const grammar::Grammar& grammar) :
    m_grammar_constructor(grammar_constructor),
    m_grammar(grammar)
{
}

void RoleCompositionGrammarVisitor::visit(RoleComposition constructor)
{
    m_result = m_grammar_constructor->get_role_or_non_terminal_left()->test_match(constructor->get_role_left(), m_grammar)
               && m_grammar_constructor->get_role_or_non_terminal_right()->test_match(constructor->get_role_right(), m_grammar);
}

/**
 * RoleTransitiveClosureGrammarVisitor
 */

RoleTransitiveClosureGrammarVisitor::RoleTransitiveClosureGrammarVisitor(grammar::RoleTransitiveClosure grammar_constructor, const grammar::Grammar& grammar) :
    m_grammar_constructor(grammar_constructor),
    m_grammar(grammar)
{
}

void RoleTransitiveClosureGrammarVisitor::visit(RoleTransitiveClosure constructor)
{
    m_result = m_grammar_constructor->get_role_or_non_terminal()->test_match(constructor->get_role(), m_grammar);
}

/**
 * RoleReflexiveTransitiveClosureGrammarVisitor
 */

RoleReflexiveTransitiveClosureGrammarVisitor::RoleReflexiveTransitiveClosureGrammarVisitor(grammar::RoleReflexiveTransitiveClosure grammar_constructor,
                                                                                           const grammar::Grammar& grammar) :
    m_grammar_constructor(grammar_constructor),
    m_grammar(grammar)
{
}

void RoleReflexiveTransitiveClosureGrammarVisitor::visit(RoleReflexiveTransitiveClosure constructor)
{
    m_result = m_grammar_constructor->get_role_or_non_terminal()->test_match(constructor->get_role(), m_grammar);
}

/**
 * RoleRestrictionGrammarVisitor
 */

RoleRestrictionGrammarVisitor::RoleRestrictionGrammarVisitor(grammar::RoleRestriction grammar_constructor, const grammar::Grammar& grammar) :
    m_grammar_constructor(grammar_constructor),
    m_grammar(grammar)
{
}

void RoleRestrictionGrammarVisitor::visit(RoleRestriction constructor)
{
    m_result = m_grammar_constructor->get_role_or_non_terminal()->test_match(constructor->get_role(), m_grammar)
               && m_grammar_constructor->get_concept_or_non_terminal()->test_match(constructor->get_concept(), m_grammar);
}

/**
 * RoleIdentityGrammarVisitor
 */

RoleIdentityGrammarVisitor::RoleIdentityGrammarVisitor(grammar::RoleIdentity grammar_constructor, const grammar::Grammar& grammar) :
    m_grammar_constructor(grammar_constructor),
    m_grammar(grammar)
{
}

void RoleIdentityGrammarVisitor::visit(RoleIdentity constructor)
{
    m_result = m_grammar_constructor->get_concept_or_non_terminal()->test_match(constructor->get_concept(), m_grammar);
}

}