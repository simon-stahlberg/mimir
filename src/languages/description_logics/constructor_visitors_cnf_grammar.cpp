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

#include "mimir/languages/description_logics/constructor_visitors_cnf_grammar.hpp"

#include "mimir/languages/description_logics/cnf_grammar_constructors.hpp"
#include "mimir/languages/description_logics/constructors.hpp"

namespace mimir::dl
{

/**
 * ConceptBotGrammarVisitor
 */

ConceptBotGrammarVisitor::ConceptBotGrammarVisitor(cnf_grammar::ConceptBot grammar_constructor, const cnf_grammar::Grammar& grammar) :
    m_grammar_constructor(grammar_constructor),
    m_grammar(grammar)
{
}

void ConceptBotGrammarVisitor::visit(ConceptBot constructor) { m_result = true; }

/**
 * ConceptTopGrammarVisitor
 */

ConceptTopGrammarVisitor::ConceptTopGrammarVisitor(cnf_grammar::ConceptTop grammar_constructor, const cnf_grammar::Grammar& grammar) :
    m_grammar_constructor(grammar_constructor),
    m_grammar(grammar)
{
}

void ConceptTopGrammarVisitor::visit(ConceptTop constructor) { m_result = true; }

/**
 * ConceptAtomicStateGrammarVisitor
 */

template<StaticOrFluentOrDerived P>
ConceptAtomicStateGrammarVisitor<P>::ConceptAtomicStateGrammarVisitor(cnf_grammar::ConceptAtomicState<P> grammar_constructor,
                                                                      const cnf_grammar::Grammar& grammar) :
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
ConceptAtomicGoalGrammarVisitor<P>::ConceptAtomicGoalGrammarVisitor(cnf_grammar::ConceptAtomicGoal<P> grammar_constructor,
                                                                    const cnf_grammar::Grammar& grammar) :
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

ConceptIntersectionGrammarVisitor::ConceptIntersectionGrammarVisitor(cnf_grammar::ConceptIntersection grammar_constructor,
                                                                     const cnf_grammar::Grammar& grammar) :
    m_grammar_constructor(grammar_constructor),
    m_grammar(grammar)
{
}

void ConceptIntersectionGrammarVisitor::visit(ConceptIntersection constructor)
{
    m_result = m_grammar_constructor->get_concept_left()->test_match(constructor->get_concept_left(), m_grammar)  //
               && m_grammar_constructor->get_concept_right()->test_match(constructor->get_concept_right(), m_grammar);
}

/**
 * ConceptUnionGrammarVisitor
 */

ConceptUnionGrammarVisitor::ConceptUnionGrammarVisitor(cnf_grammar::ConceptUnion grammar_constructor, const cnf_grammar::Grammar& grammar) :
    m_grammar_constructor(grammar_constructor),
    m_grammar(grammar)
{
}

void ConceptUnionGrammarVisitor::visit(ConceptUnion constructor)
{
    m_result = m_grammar_constructor->get_concept_left()->test_match(constructor->get_concept_left(), m_grammar)  //
               && m_grammar_constructor->get_concept_right()->test_match(constructor->get_concept_right(), m_grammar);
}

/**
 * ConceptNegationGrammarVisitor
 */

ConceptNegationGrammarVisitor::ConceptNegationGrammarVisitor(cnf_grammar::ConceptNegation grammar_constructor, const cnf_grammar::Grammar& grammar) :
    m_grammar_constructor(grammar_constructor),
    m_grammar(grammar)
{
}

void ConceptNegationGrammarVisitor::visit(ConceptNegation constructor)
{
    m_result = m_grammar_constructor->get_concept()->test_match(constructor->get_concept(), m_grammar);
}

/**
 * ConceptValueRestrictionGrammarVisitor
 */

ConceptValueRestrictionGrammarVisitor::ConceptValueRestrictionGrammarVisitor(cnf_grammar::ConceptValueRestriction grammar_constructor,
                                                                             const cnf_grammar::Grammar& grammar) :
    m_grammar_constructor(grammar_constructor),
    m_grammar(grammar)
{
}

void ConceptValueRestrictionGrammarVisitor::visit(ConceptValueRestriction constructor)
{
    m_result = m_grammar_constructor->get_role()->test_match(constructor->get_role(), m_grammar)  //
               && m_grammar_constructor->get_concept()->test_match(constructor->get_concept(), m_grammar);
}

/**
 * ConceptExistentialQuantificationGrammarVisitor
 */

ConceptExistentialQuantificationGrammarVisitor::ConceptExistentialQuantificationGrammarVisitor(
    cnf_grammar::ConceptExistentialQuantification grammar_constructor,
    const cnf_grammar::Grammar& grammar) :
    m_grammar_constructor(grammar_constructor),
    m_grammar(grammar)
{
}

void ConceptExistentialQuantificationGrammarVisitor::visit(ConceptExistentialQuantification constructor)
{
    m_result = m_grammar_constructor->get_role()->test_match(constructor->get_role(), m_grammar)  //
               && m_grammar_constructor->get_concept()->test_match(constructor->get_concept(), m_grammar);
}

/**
 * ConceptRoleValueMapContainmentGrammarVisitor
 */

ConceptRoleValueMapContainmentGrammarVisitor::ConceptRoleValueMapContainmentGrammarVisitor(cnf_grammar::ConceptRoleValueMapContainment grammar_constructor,
                                                                                           const cnf_grammar::Grammar& grammar) :
    m_grammar_constructor(grammar_constructor),
    m_grammar(grammar)
{
}

void ConceptRoleValueMapContainmentGrammarVisitor::visit(ConceptRoleValueMapContainment constructor)
{
    m_result = m_grammar_constructor->get_role_left()->test_match(constructor->get_role_left(), m_grammar)  //
               && m_grammar_constructor->get_role_right()->test_match(constructor->get_role_right(), m_grammar);
}

/**
 * ConceptRoleValueMapEqualityGrammarVisitor
 */

ConceptRoleValueMapEqualityGrammarVisitor::ConceptRoleValueMapEqualityGrammarVisitor(cnf_grammar::ConceptRoleValueMapEquality grammar_constructor,
                                                                                     const cnf_grammar::Grammar& grammar) :
    m_grammar_constructor(grammar_constructor),
    m_grammar(grammar)
{
}

void ConceptRoleValueMapEqualityGrammarVisitor::visit(ConceptRoleValueMapEquality constructor)
{
    m_result = m_grammar_constructor->get_role_left()->test_match(constructor->get_role_left(), m_grammar)  //
               && m_grammar_constructor->get_role_right()->test_match(constructor->get_role_right(), m_grammar);
}

/**
 * ConceptNominalGrammarVisitor
 */

ConceptNominalGrammarVisitor::ConceptNominalGrammarVisitor(cnf_grammar::ConceptNominal grammar_constructor, const cnf_grammar::Grammar& grammar) :
    m_grammar_constructor(grammar_constructor),
    m_grammar(grammar)
{
}

void ConceptNominalGrammarVisitor::visit(ConceptNominal constructor) { m_result = m_grammar_constructor->get_object() == constructor->get_object(); }

/**
 * RoleUniversalGrammarVisitor
 */

RoleUniversalGrammarVisitor::RoleUniversalGrammarVisitor(cnf_grammar::RoleUniversal grammar_constructor, const cnf_grammar::Grammar& grammar) :
    m_grammar_constructor(grammar_constructor),
    m_grammar(grammar)
{
}

void RoleUniversalGrammarVisitor::visit(RoleUniversal constructor) { m_result = true; }

/**
 * RoleAtomicStateGrammarVisitor
 */

template<StaticOrFluentOrDerived P>
RoleAtomicStateGrammarVisitor<P>::RoleAtomicStateGrammarVisitor(cnf_grammar::RoleAtomicState<P> grammar_constructor, const cnf_grammar::Grammar& grammar) :
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
RoleAtomicGoalGrammarVisitor<P>::RoleAtomicGoalGrammarVisitor(cnf_grammar::RoleAtomicGoal<P> grammar_constructor, const cnf_grammar::Grammar& grammar) :
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

RoleIntersectionGrammarVisitor::RoleIntersectionGrammarVisitor(cnf_grammar::RoleIntersection grammar_constructor, const cnf_grammar::Grammar& grammar) :
    m_grammar_constructor(grammar_constructor),
    m_grammar(grammar)
{
}

void RoleIntersectionGrammarVisitor::visit(RoleIntersection constructor)
{
    m_result = m_grammar_constructor->get_role_left()->test_match(constructor->get_role_left(), m_grammar)
               && m_grammar_constructor->get_role_right()->test_match(constructor->get_role_right(), m_grammar);
}

/**
 * RoleUnionGrammarVisitor
 */

RoleUnionGrammarVisitor::RoleUnionGrammarVisitor(cnf_grammar::RoleUnion grammar_constructor, const cnf_grammar::Grammar& grammar) :
    m_grammar_constructor(grammar_constructor),
    m_grammar(grammar)
{
}

void RoleUnionGrammarVisitor::visit(RoleUnion constructor)
{
    m_result = m_grammar_constructor->get_role_left()->test_match(constructor->get_role_left(), m_grammar)
               && m_grammar_constructor->get_role_right()->test_match(constructor->get_role_right(), m_grammar);
}

/**
 * RoleComplementGrammarVisitor
 */

RoleComplementGrammarVisitor::RoleComplementGrammarVisitor(cnf_grammar::RoleComplement grammar_constructor, const cnf_grammar::Grammar& grammar) :
    m_grammar_constructor(grammar_constructor),
    m_grammar(grammar)
{
}

void RoleComplementGrammarVisitor::visit(RoleComplement constructor)
{
    m_result = m_grammar_constructor->get_role()->test_match(constructor->get_role(), m_grammar);
}

/**
 * RoleInverseGrammarVisitor
 */

RoleInverseGrammarVisitor::RoleInverseGrammarVisitor(cnf_grammar::RoleInverse grammar_constructor, const cnf_grammar::Grammar& grammar) :
    m_grammar_constructor(grammar_constructor),
    m_grammar(grammar)
{
}

void RoleInverseGrammarVisitor::visit(RoleInverse constructor) { m_result = m_grammar_constructor->get_role()->test_match(constructor->get_role(), m_grammar); }

/**
 * RoleCompositionGrammarVisitor
 */

RoleCompositionGrammarVisitor::RoleCompositionGrammarVisitor(cnf_grammar::RoleComposition grammar_constructor, const cnf_grammar::Grammar& grammar) :
    m_grammar_constructor(grammar_constructor),
    m_grammar(grammar)
{
}

void RoleCompositionGrammarVisitor::visit(RoleComposition constructor)
{
    m_result = m_grammar_constructor->get_role_left()->test_match(constructor->get_role_left(), m_grammar)
               && m_grammar_constructor->get_role_right()->test_match(constructor->get_role_right(), m_grammar);
}

/**
 * RoleTransitiveClosureGrammarVisitor
 */

RoleTransitiveClosureGrammarVisitor::RoleTransitiveClosureGrammarVisitor(cnf_grammar::RoleTransitiveClosure grammar_constructor,
                                                                         const cnf_grammar::Grammar& grammar) :
    m_grammar_constructor(grammar_constructor),
    m_grammar(grammar)
{
}

void RoleTransitiveClosureGrammarVisitor::visit(RoleTransitiveClosure constructor)
{
    m_result = m_grammar_constructor->get_role()->test_match(constructor->get_role(), m_grammar);
}

/**
 * RoleReflexiveTransitiveClosureGrammarVisitor
 */

RoleReflexiveTransitiveClosureGrammarVisitor::RoleReflexiveTransitiveClosureGrammarVisitor(cnf_grammar::RoleReflexiveTransitiveClosure grammar_constructor,
                                                                                           const cnf_grammar::Grammar& grammar) :
    m_grammar_constructor(grammar_constructor),
    m_grammar(grammar)
{
}

void RoleReflexiveTransitiveClosureGrammarVisitor::visit(RoleReflexiveTransitiveClosure constructor)
{
    m_result = m_grammar_constructor->get_role()->test_match(constructor->get_role(), m_grammar);
}

/**
 * RoleRestrictionGrammarVisitor
 */

RoleRestrictionGrammarVisitor::RoleRestrictionGrammarVisitor(cnf_grammar::RoleRestriction grammar_constructor, const cnf_grammar::Grammar& grammar) :
    m_grammar_constructor(grammar_constructor),
    m_grammar(grammar)
{
}

void RoleRestrictionGrammarVisitor::visit(RoleRestriction constructor)
{
    m_result = m_grammar_constructor->get_role()->test_match(constructor->get_role(), m_grammar)
               && m_grammar_constructor->get_concept()->test_match(constructor->get_concept(), m_grammar);
}

/**
 * RoleIdentityGrammarVisitor
 */

RoleIdentityGrammarVisitor::RoleIdentityGrammarVisitor(cnf_grammar::RoleIdentity grammar_constructor, const cnf_grammar::Grammar& grammar) :
    m_grammar_constructor(grammar_constructor),
    m_grammar(grammar)
{
}

void RoleIdentityGrammarVisitor::visit(RoleIdentity constructor)
{
    m_result = m_grammar_constructor->get_concept()->test_match(constructor->get_concept(), m_grammar);
}

}