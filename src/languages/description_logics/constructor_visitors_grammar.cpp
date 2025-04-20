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

#include "mimir/languages/description_logics/constructor_visitor_grammar.hpp"
#include "mimir/languages/description_logics/constructors.hpp"
#include "mimir/languages/description_logics/grammar_constructors.hpp"

using namespace mimir::formalism;

namespace mimir::languages::dl::grammar
{

/**
 * ConceptBotGrammarVisitor
 */

ConceptBotGrammarVisitor::ConceptBotGrammarVisitor(ConceptBot grammar_constructor, const Grammar& grammar) :
    m_grammar_constructor(grammar_constructor),
    m_grammar(grammar)
{
}

void ConceptBotGrammarVisitor::visit(dl::ConceptBot constructor) { m_result = true; }

/**
 * ConceptTopGrammarVisitor
 */

ConceptTopGrammarVisitor::ConceptTopGrammarVisitor(ConceptTop grammar_constructor, const Grammar& grammar) :
    m_grammar_constructor(grammar_constructor),
    m_grammar(grammar)
{
}

void ConceptTopGrammarVisitor::visit(dl::ConceptTop constructor) { m_result = true; }

/**
 * ConceptAtomicStateGrammarVisitor
 */

template<IsStaticOrFluentOrDerivedTag P>
ConceptAtomicStateGrammarVisitor<P>::ConceptAtomicStateGrammarVisitor(ConceptAtomicState<P> grammar_constructor, const Grammar& grammar) :
    m_grammar_constructor(grammar_constructor),
    m_grammar(grammar)
{
}

template<IsStaticOrFluentOrDerivedTag P>
void ConceptAtomicStateGrammarVisitor<P>::visit(dl::ConceptAtomicState<P> constructor)
{
    m_result = constructor->get_predicate() == m_grammar_constructor->get_predicate();
}

template class ConceptAtomicStateGrammarVisitor<StaticTag>;
template class ConceptAtomicStateGrammarVisitor<FluentTag>;
template class ConceptAtomicStateGrammarVisitor<DerivedTag>;

/**
 * ConceptAtomicGoalGrammarVisitor
 */

template<IsStaticOrFluentOrDerivedTag P>
ConceptAtomicGoalGrammarVisitor<P>::ConceptAtomicGoalGrammarVisitor(ConceptAtomicGoal<P> grammar_constructor, const Grammar& grammar) :
    m_grammar_constructor(grammar_constructor),
    m_grammar(grammar)
{
}

template<IsStaticOrFluentOrDerivedTag P>
void ConceptAtomicGoalGrammarVisitor<P>::visit(dl::ConceptAtomicGoal<P> constructor)
{
    m_result = constructor->get_predicate() == m_grammar_constructor->get_predicate();
}

template class ConceptAtomicGoalGrammarVisitor<StaticTag>;
template class ConceptAtomicGoalGrammarVisitor<FluentTag>;
template class ConceptAtomicGoalGrammarVisitor<DerivedTag>;

/**
 * ConceptIntersectionGrammarVisitor
 */

ConceptIntersectionGrammarVisitor::ConceptIntersectionGrammarVisitor(ConceptIntersection grammar_constructor, const Grammar& grammar) :
    m_grammar_constructor(grammar_constructor),
    m_grammar(grammar)
{
}

void ConceptIntersectionGrammarVisitor::visit(dl::ConceptIntersection constructor)
{
    m_result = m_grammar_constructor->get_left_concept_or_non_terminal()->test_match(constructor->get_left_concept(), m_grammar)  //
               && m_grammar_constructor->get_right_concept_or_non_terminal()->test_match(constructor->get_right_concept(), m_grammar);
}

/**
 * ConceptUnionGrammarVisitor
 */

ConceptUnionGrammarVisitor::ConceptUnionGrammarVisitor(ConceptUnion grammar_constructor, const Grammar& grammar) :
    m_grammar_constructor(grammar_constructor),
    m_grammar(grammar)
{
}

void ConceptUnionGrammarVisitor::visit(dl::ConceptUnion constructor)
{
    m_result = m_grammar_constructor->get_left_concept_or_non_terminal()->test_match(constructor->get_left_concept(), m_grammar)  //
               && m_grammar_constructor->get_right_concept_or_non_terminal()->test_match(constructor->get_right_concept(), m_grammar);
}

/**
 * ConceptNegationGrammarVisitor
 */

ConceptNegationGrammarVisitor::ConceptNegationGrammarVisitor(ConceptNegation grammar_constructor, const Grammar& grammar) :
    m_grammar_constructor(grammar_constructor),
    m_grammar(grammar)
{
}

void ConceptNegationGrammarVisitor::visit(dl::ConceptNegation constructor)
{
    m_result = m_grammar_constructor->get_concept_or_non_terminal()->test_match(constructor->get_concept(), m_grammar);
}

/**
 * ConceptValueRestrictionGrammarVisitor
 */

ConceptValueRestrictionGrammarVisitor::ConceptValueRestrictionGrammarVisitor(ConceptValueRestriction grammar_constructor, const Grammar& grammar) :
    m_grammar_constructor(grammar_constructor),
    m_grammar(grammar)
{
}

void ConceptValueRestrictionGrammarVisitor::visit(dl::ConceptValueRestriction constructor)
{
    m_result = m_grammar_constructor->get_role_or_non_terminal()->test_match(constructor->get_role(), m_grammar)  //
               && m_grammar_constructor->get_concept_or_non_terminal()->test_match(constructor->get_concept(), m_grammar);
}

/**
 * ConceptExistentialQuantificationGrammarVisitor
 */

ConceptExistentialQuantificationGrammarVisitor::ConceptExistentialQuantificationGrammarVisitor(ConceptExistentialQuantification grammar_constructor,
                                                                                               const Grammar& grammar) :
    m_grammar_constructor(grammar_constructor),
    m_grammar(grammar)
{
}

void ConceptExistentialQuantificationGrammarVisitor::visit(dl::ConceptExistentialQuantification constructor)
{
    m_result = m_grammar_constructor->get_role_or_non_terminal()->test_match(constructor->get_role(), m_grammar)  //
               && m_grammar_constructor->get_concept_or_non_terminal()->test_match(constructor->get_concept(), m_grammar);
}

/**
 * ConceptRoleValueMapContainmentGrammarVisitor
 */

ConceptRoleValueMapContainmentGrammarVisitor::ConceptRoleValueMapContainmentGrammarVisitor(ConceptRoleValueMapContainment grammar_constructor,
                                                                                           const Grammar& grammar) :
    m_grammar_constructor(grammar_constructor),
    m_grammar(grammar)
{
}

void ConceptRoleValueMapContainmentGrammarVisitor::visit(dl::ConceptRoleValueMapContainment constructor)
{
    m_result = m_grammar_constructor->get_left_role_or_non_terminal()->test_match(constructor->get_left_role(), m_grammar)  //
               && m_grammar_constructor->get_right_role_or_non_terminal()->test_match(constructor->get_right_role(), m_grammar);
}

/**
 * ConceptRoleValueMapEqualityGrammarVisitor
 */

ConceptRoleValueMapEqualityGrammarVisitor::ConceptRoleValueMapEqualityGrammarVisitor(ConceptRoleValueMapEquality grammar_constructor, const Grammar& grammar) :
    m_grammar_constructor(grammar_constructor),
    m_grammar(grammar)
{
}

void ConceptRoleValueMapEqualityGrammarVisitor::visit(dl::ConceptRoleValueMapEquality constructor)
{
    m_result = m_grammar_constructor->get_left_role_or_non_terminal()->test_match(constructor->get_left_role(), m_grammar)  //
               && m_grammar_constructor->get_right_role_or_non_terminal()->test_match(constructor->get_right_role(), m_grammar);
}

/**
 * ConceptNominalGrammarVisitor
 */

ConceptNominalGrammarVisitor::ConceptNominalGrammarVisitor(ConceptNominal grammar_constructor, const Grammar& grammar) :
    m_grammar_constructor(grammar_constructor),
    m_grammar(grammar)
{
}

void ConceptNominalGrammarVisitor::visit(dl::ConceptNominal constructor) { m_result = m_grammar_constructor->get_object() == constructor->get_object(); }

/**
 * RoleUniversalGrammarVisitor
 */

RoleUniversalGrammarVisitor::RoleUniversalGrammarVisitor(RoleUniversal grammar_constructor, const Grammar& grammar) :
    m_grammar_constructor(grammar_constructor),
    m_grammar(grammar)
{
}

void RoleUniversalGrammarVisitor::visit(dl::RoleUniversal constructor) { m_result = true; }

/**
 * RoleAtomicStateGrammarVisitor
 */

template<IsStaticOrFluentOrDerivedTag P>
RoleAtomicStateGrammarVisitor<P>::RoleAtomicStateGrammarVisitor(RoleAtomicState<P> grammar_constructor, const Grammar& grammar) :
    m_grammar_constructor(grammar_constructor),
    m_grammar(grammar)
{
}

template<IsStaticOrFluentOrDerivedTag P>
void RoleAtomicStateGrammarVisitor<P>::visit(dl::RoleAtomicState<P> constructor)
{
    m_result = constructor->get_predicate() == m_grammar_constructor->get_predicate();
}

template class RoleAtomicStateGrammarVisitor<StaticTag>;
template class RoleAtomicStateGrammarVisitor<FluentTag>;
template class RoleAtomicStateGrammarVisitor<DerivedTag>;

/**
 * RoleAtomicGoalGrammarVisitor
 */

template<IsStaticOrFluentOrDerivedTag P>
RoleAtomicGoalGrammarVisitor<P>::RoleAtomicGoalGrammarVisitor(RoleAtomicGoal<P> grammar_constructor, const Grammar& grammar) :
    m_grammar_constructor(grammar_constructor),
    m_grammar(grammar)
{
}

template<IsStaticOrFluentOrDerivedTag P>
void RoleAtomicGoalGrammarVisitor<P>::visit(dl::RoleAtomicGoal<P> constructor)
{
    m_result = (constructor->get_predicate() == m_grammar_constructor->get_predicate());
}

template class RoleAtomicGoalGrammarVisitor<StaticTag>;
template class RoleAtomicGoalGrammarVisitor<FluentTag>;
template class RoleAtomicGoalGrammarVisitor<DerivedTag>;

/**
 * RoleIntersectionGrammarVisitor
 */

RoleIntersectionGrammarVisitor::RoleIntersectionGrammarVisitor(RoleIntersection grammar_constructor, const Grammar& grammar) :
    m_grammar_constructor(grammar_constructor),
    m_grammar(grammar)
{
}

void RoleIntersectionGrammarVisitor::visit(dl::RoleIntersection constructor)
{
    m_result = m_grammar_constructor->get_left_role_or_non_terminal()->test_match(constructor->get_left_role(), m_grammar)
               && m_grammar_constructor->get_right_role_or_non_terminal()->test_match(constructor->get_right_role(), m_grammar);
}

/**
 * RoleUnionGrammarVisitor
 */

RoleUnionGrammarVisitor::RoleUnionGrammarVisitor(RoleUnion grammar_constructor, const Grammar& grammar) :
    m_grammar_constructor(grammar_constructor),
    m_grammar(grammar)
{
}

void RoleUnionGrammarVisitor::visit(dl::RoleUnion constructor)
{
    m_result = m_grammar_constructor->get_left_role_or_non_terminal()->test_match(constructor->get_left_role(), m_grammar)
               && m_grammar_constructor->get_right_role_or_non_terminal()->test_match(constructor->get_right_role(), m_grammar);
}

/**
 * RoleComplementGrammarVisitor
 */

RoleComplementGrammarVisitor::RoleComplementGrammarVisitor(RoleComplement grammar_constructor, const Grammar& grammar) :
    m_grammar_constructor(grammar_constructor),
    m_grammar(grammar)
{
}

void RoleComplementGrammarVisitor::visit(dl::RoleComplement constructor)
{
    m_result = m_grammar_constructor->get_role_or_non_terminal()->test_match(constructor->get_role(), m_grammar);
}

/**
 * RoleInverseGrammarVisitor
 */

RoleInverseGrammarVisitor::RoleInverseGrammarVisitor(RoleInverse grammar_constructor, const Grammar& grammar) :
    m_grammar_constructor(grammar_constructor),
    m_grammar(grammar)
{
}

void RoleInverseGrammarVisitor::visit(dl::RoleInverse constructor)
{
    m_result = m_grammar_constructor->get_role_or_non_terminal()->test_match(constructor->get_role(), m_grammar);
}

/**
 * RoleCompositionGrammarVisitor
 */

RoleCompositionGrammarVisitor::RoleCompositionGrammarVisitor(RoleComposition grammar_constructor, const Grammar& grammar) :
    m_grammar_constructor(grammar_constructor),
    m_grammar(grammar)
{
}

void RoleCompositionGrammarVisitor::visit(dl::RoleComposition constructor)
{
    m_result = m_grammar_constructor->get_left_role_or_non_terminal()->test_match(constructor->get_left_role(), m_grammar)
               && m_grammar_constructor->get_right_role_or_non_terminal()->test_match(constructor->get_right_role(), m_grammar);
}

/**
 * RoleTransitiveClosureGrammarVisitor
 */

RoleTransitiveClosureGrammarVisitor::RoleTransitiveClosureGrammarVisitor(RoleTransitiveClosure grammar_constructor, const Grammar& grammar) :
    m_grammar_constructor(grammar_constructor),
    m_grammar(grammar)
{
}

void RoleTransitiveClosureGrammarVisitor::visit(dl::RoleTransitiveClosure constructor)
{
    m_result = m_grammar_constructor->get_role_or_non_terminal()->test_match(constructor->get_role(), m_grammar);
}

/**
 * RoleReflexiveTransitiveClosureGrammarVisitor
 */

RoleReflexiveTransitiveClosureGrammarVisitor::RoleReflexiveTransitiveClosureGrammarVisitor(RoleReflexiveTransitiveClosure grammar_constructor,
                                                                                           const Grammar& grammar) :
    m_grammar_constructor(grammar_constructor),
    m_grammar(grammar)
{
}

void RoleReflexiveTransitiveClosureGrammarVisitor::visit(dl::RoleReflexiveTransitiveClosure constructor)
{
    m_result = m_grammar_constructor->get_role_or_non_terminal()->test_match(constructor->get_role(), m_grammar);
}

/**
 * RoleRestrictionGrammarVisitor
 */

RoleRestrictionGrammarVisitor::RoleRestrictionGrammarVisitor(RoleRestriction grammar_constructor, const Grammar& grammar) :
    m_grammar_constructor(grammar_constructor),
    m_grammar(grammar)
{
}

void RoleRestrictionGrammarVisitor::visit(dl::RoleRestriction constructor)
{
    m_result = m_grammar_constructor->get_role_or_non_terminal()->test_match(constructor->get_role(), m_grammar)
               && m_grammar_constructor->get_concept_or_non_terminal()->test_match(constructor->get_concept(), m_grammar);
}

/**
 * RoleIdentityGrammarVisitor
 */

RoleIdentityGrammarVisitor::RoleIdentityGrammarVisitor(RoleIdentity grammar_constructor, const Grammar& grammar) :
    m_grammar_constructor(grammar_constructor),
    m_grammar(grammar)
{
}

void RoleIdentityGrammarVisitor::visit(dl::RoleIdentity constructor)
{
    m_result = m_grammar_constructor->get_concept_or_non_terminal()->test_match(constructor->get_concept(), m_grammar);
}

/**
 * BooleanVisitors
 */

template<IsStaticOrFluentOrDerivedTag P>
BooleanAtomicStateGrammarVisitor<P>::BooleanAtomicStateGrammarVisitor(BooleanAtomicState<P> grammar_constructor, const Grammar& grammar) :
    m_grammar_constructor(grammar_constructor),
    m_grammar(grammar)
{
}

template<IsStaticOrFluentOrDerivedTag P>
void BooleanAtomicStateGrammarVisitor<P>::visit(dl::BooleanAtomicState<P> constructor)
{
    m_result = (m_grammar_constructor->get_predicate() == constructor->get_predicate());
}

template class BooleanAtomicStateGrammarVisitor<StaticTag>;
template class BooleanAtomicStateGrammarVisitor<FluentTag>;
template class BooleanAtomicStateGrammarVisitor<DerivedTag>;

template<IsConceptOrRoleTag D>
BooleanNonemptyGrammarVisitor<D>::BooleanNonemptyGrammarVisitor(BooleanNonempty<D> grammar_constructor, const Grammar& grammar) :
    m_grammar_constructor(grammar_constructor),
    m_grammar(grammar)
{
}

template<IsConceptOrRoleTag D>
void BooleanNonemptyGrammarVisitor<D>::visit(dl::BooleanNonempty<D> constructor)
{
    m_result = m_grammar_constructor->get_constructor_or_nonterminal()->test_match(constructor->get_constructor(), m_grammar);
}

template class BooleanNonemptyGrammarVisitor<ConceptTag>;
template class BooleanNonemptyGrammarVisitor<RoleTag>;

/**
 * NumericalVisitors
 */

template<IsConceptOrRoleTag D>
NumericalCountGrammarVisitor<D>::NumericalCountGrammarVisitor(NumericalCount<D> grammar_constructor, const Grammar& grammar) :
    m_grammar_constructor(grammar_constructor),
    m_grammar(grammar)
{
}

template<IsConceptOrRoleTag D>
void NumericalCountGrammarVisitor<D>::visit(dl::NumericalCount<D> constructor)
{
    m_result = m_grammar_constructor->get_constructor_or_nonterminal()->test_match(constructor->get_constructor(), m_grammar);
}

template class NumericalCountGrammarVisitor<ConceptTag>;
template class NumericalCountGrammarVisitor<RoleTag>;

NumericalDistanceGrammarVisitor::NumericalDistanceGrammarVisitor(NumericalDistance grammar_constructor, const Grammar& grammar) :
    m_grammar_constructor(grammar_constructor),
    m_grammar(grammar)
{
}

void NumericalDistanceGrammarVisitor::visit(dl::NumericalDistance constructor)
{
    m_result = m_grammar_constructor->get_left_concept_or_nonterminal()->test_match(constructor->get_left_concept(), m_grammar)
               && m_grammar_constructor->get_role_or_nonterminal()->test_match(constructor->get_role(), m_grammar)
               && m_grammar_constructor->get_right_concept_or_nonterminal()->test_match(constructor->get_right_concept(), m_grammar);
}

}