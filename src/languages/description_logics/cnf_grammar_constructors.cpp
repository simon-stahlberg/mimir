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

#include "mimir/languages/description_logics/cnf_grammar_constructors.hpp"

#include "mimir/formalism/predicate.hpp"
#include "mimir/languages/description_logics/cnf_grammar.hpp"
#include "mimir/languages/description_logics/cnf_grammar_visitor_interface.hpp"
#include "mimir/languages/description_logics/constructor_visitors_cnf_grammar.hpp"

namespace mimir::dl::cnf_grammar
{

/**
 * NonTerminal
 */

template<dl::FeatureCategory D>
NonTerminalImpl<D>::NonTerminalImpl(Index index, std::string name) : m_index(index), m_name(std::move(name))
{
}

template<dl::FeatureCategory D>
bool NonTerminalImpl<D>::test_match(dl::Constructor<D> constructor, const Grammar& grammar) const
{
    const auto& derivation_rules = grammar.get_derivation_rules_container().get(this);
    const auto& substitution_rules = grammar.get_substitution_rules_container().get(this);

    return std::any_of(derivation_rules.begin(), derivation_rules.end(), [&](auto&& rule) { return rule->test_match(constructor, grammar); })
           || std::any_of(substitution_rules.begin(), substitution_rules.end(), [&](auto&& rule) { return rule->test_match(constructor, grammar); });
}

template<dl::FeatureCategory D>
void NonTerminalImpl<D>::accept(Visitor& visitor) const
{
    visitor.visit(this);
}

template<dl::FeatureCategory D>
Index NonTerminalImpl<D>::get_index() const
{
    return m_index;
}

template<dl::FeatureCategory D>
const std::string& NonTerminalImpl<D>::get_name() const
{
    return m_name;
}

template class NonTerminalImpl<Concept>;
template class NonTerminalImpl<Role>;
template class NonTerminalImpl<Boolean>;
template class NonTerminalImpl<Numerical>;

/**
 * DerivationRule
 */

template<dl::FeatureCategory D>
DerivationRuleImpl<D>::DerivationRuleImpl(Index index, NonTerminal<D> head, Constructor<D> body) : m_index(index), m_head(head), m_body(body)
{
}

template<dl::FeatureCategory D>
bool DerivationRuleImpl<D>::test_match(dl::Constructor<D> constructor, const Grammar& grammar) const
{
    return m_body->test_match(constructor, grammar);
}

template<dl::FeatureCategory D>
void DerivationRuleImpl<D>::accept(Visitor& visitor) const
{
    visitor.visit(this);
}

template<dl::FeatureCategory D>
Index DerivationRuleImpl<D>::get_index() const
{
    return m_index;
}

template<dl::FeatureCategory D>
const NonTerminal<D>& DerivationRuleImpl<D>::get_head() const
{
    return m_head;
}

template<dl::FeatureCategory D>
const Constructor<D>& DerivationRuleImpl<D>::get_body() const
{
    return m_body;
}

template class DerivationRuleImpl<Concept>;
template class DerivationRuleImpl<Role>;
template class DerivationRuleImpl<Boolean>;
template class DerivationRuleImpl<Numerical>;

/**
 * SubstitutionRule
 */

template<dl::FeatureCategory D>
SubstitutionRuleImpl<D>::SubstitutionRuleImpl(Index index, NonTerminal<D> head, NonTerminal<D> body) : m_index(index), m_head(head), m_body(body)
{
}

template<dl::FeatureCategory D>
bool SubstitutionRuleImpl<D>::test_match(dl::Constructor<D> constructor, const Grammar& grammar) const
{
    return m_body->test_match(constructor, grammar);
}

template<dl::FeatureCategory D>
void SubstitutionRuleImpl<D>::accept(Visitor& visitor) const
{
    visitor.visit(this);
}

template<dl::FeatureCategory D>
Index SubstitutionRuleImpl<D>::get_index() const
{
    return m_index;
}

template<dl::FeatureCategory D>
const NonTerminal<D>& SubstitutionRuleImpl<D>::get_head() const
{
    return m_head;
}

template<dl::FeatureCategory D>
const NonTerminal<D>& SubstitutionRuleImpl<D>::get_body() const
{
    return m_body;
}

template class SubstitutionRuleImpl<Concept>;
template class SubstitutionRuleImpl<Role>;
template class SubstitutionRuleImpl<Boolean>;
template class SubstitutionRuleImpl<Numerical>;

/**
 * ConceptBot
 */

ConceptBotImpl::ConceptBotImpl(Index index) : m_index(index) {}

bool ConceptBotImpl::test_match(dl::Constructor<Concept> constructor, const Grammar& grammar) const
{
    auto visitor = ConceptBotGrammarVisitor(this, grammar);
    constructor->accept(visitor);
    return visitor.get_result();
}

void ConceptBotImpl::accept(Visitor& visitor) const { visitor.visit(this); }

Index ConceptBotImpl::get_index() const { return m_index; }

/**
 * ConceptTop
 */

ConceptTopImpl::ConceptTopImpl(Index index) : m_index(index) {}

bool ConceptTopImpl::test_match(dl::Constructor<Concept> constructor, const Grammar& grammar) const
{
    auto visitor = ConceptTopGrammarVisitor(this, grammar);
    constructor->accept(visitor);
    return visitor.get_result();
}

void ConceptTopImpl::accept(Visitor& visitor) const { visitor.visit(this); }

Index ConceptTopImpl::get_index() const { return m_index; }

/**
 * ConceptAtomicStateImpl
 */

template<StaticOrFluentOrDerived P>
ConceptAtomicStateImpl<P>::ConceptAtomicStateImpl(Index index, Predicate<P> predicate) : m_index(index), m_predicate(predicate)
{
}

template<StaticOrFluentOrDerived P>
bool ConceptAtomicStateImpl<P>::test_match(dl::Constructor<Concept> constructor, const Grammar& grammar) const
{
    auto visitor = ConceptAtomicStateGrammarVisitor<P>(this, grammar);
    constructor->accept(visitor);
    return visitor.get_result();
}

template<StaticOrFluentOrDerived P>
void ConceptAtomicStateImpl<P>::accept(Visitor& visitor) const
{
    visitor.visit(this);
}

template<StaticOrFluentOrDerived P>
Index ConceptAtomicStateImpl<P>::get_index() const
{
    return m_index;
}

template<StaticOrFluentOrDerived P>
Predicate<P> ConceptAtomicStateImpl<P>::get_predicate() const
{
    return m_predicate;
}

template class ConceptAtomicStateImpl<Static>;
template class ConceptAtomicStateImpl<Fluent>;
template class ConceptAtomicStateImpl<Derived>;

/**
 * ConceptAtomicGoal
 */

template<StaticOrFluentOrDerived P>
ConceptAtomicGoalImpl<P>::ConceptAtomicGoalImpl(Index index, Predicate<P> predicate, bool is_negated) :
    m_index(index),

    m_predicate(predicate),
    m_is_negated(is_negated)
{
}

template<StaticOrFluentOrDerived P>
bool ConceptAtomicGoalImpl<P>::test_match(dl::Constructor<Concept> constructor, const Grammar& grammar) const
{
    auto visitor = ConceptAtomicGoalGrammarVisitor<P>(this, grammar);
    constructor->accept(visitor);
    return visitor.get_result();
}

template<StaticOrFluentOrDerived P>
void ConceptAtomicGoalImpl<P>::accept(Visitor& visitor) const
{
    visitor.visit(this);
}

template<StaticOrFluentOrDerived P>
Index ConceptAtomicGoalImpl<P>::get_index() const
{
    return m_index;
}

template<StaticOrFluentOrDerived P>
Predicate<P> ConceptAtomicGoalImpl<P>::get_predicate() const
{
    return m_predicate;
}

template<StaticOrFluentOrDerived P>
bool ConceptAtomicGoalImpl<P>::is_negated() const
{
    return m_is_negated;
}

template class ConceptAtomicGoalImpl<Static>;
template class ConceptAtomicGoalImpl<Fluent>;
template class ConceptAtomicGoalImpl<Derived>;

/**
 * ConceptIntersection
 */
ConceptIntersectionImpl::ConceptIntersectionImpl(Index index, NonTerminal<Concept> left_concept, NonTerminal<Concept> right_concept) :
    m_index(index),
    m_left_concept(left_concept),
    m_right_concept(right_concept)
{
}

bool ConceptIntersectionImpl::test_match(dl::Constructor<Concept> constructor, const Grammar& grammar) const
{
    auto visitor = ConceptIntersectionGrammarVisitor(this, grammar);
    constructor->accept(visitor);
    return visitor.get_result();
}

void ConceptIntersectionImpl::accept(Visitor& visitor) const { visitor.visit(this); }

Index ConceptIntersectionImpl::get_index() const { return m_index; }

NonTerminal<Concept> ConceptIntersectionImpl::get_left_concept() const { return m_left_concept; }

NonTerminal<Concept> ConceptIntersectionImpl::get_right_concept() const { return m_right_concept; }

/**
 * ConceptUnion
 */

ConceptUnionImpl::ConceptUnionImpl(Index index, NonTerminal<Concept> left_concept, NonTerminal<Concept> right_concept) :
    m_index(index),
    m_left_concept(left_concept),
    m_right_concept(right_concept)
{
}

bool ConceptUnionImpl::test_match(dl::Constructor<Concept> constructor, const Grammar& grammar) const
{
    auto visitor = ConceptUnionGrammarVisitor(this, grammar);
    constructor->accept(visitor);
    return visitor.get_result();
}

void ConceptUnionImpl::accept(Visitor& visitor) const { visitor.visit(this); }

Index ConceptUnionImpl::get_index() const { return m_index; }

NonTerminal<Concept> ConceptUnionImpl::get_left_concept() const { return m_left_concept; }

NonTerminal<Concept> ConceptUnionImpl::get_right_concept() const { return m_right_concept; }

/**
 * ConceptNegation
 */

ConceptNegationImpl::ConceptNegationImpl(Index index, NonTerminal<Concept> concept_) : m_index(index), m_concept(concept_) {}

bool ConceptNegationImpl::test_match(dl::Constructor<Concept> constructor, const Grammar& grammar) const
{
    auto visitor = ConceptNegationGrammarVisitor(this, grammar);
    constructor->accept(visitor);
    return visitor.get_result();
}

void ConceptNegationImpl::accept(Visitor& visitor) const { visitor.visit(this); }

Index ConceptNegationImpl::get_index() const { return m_index; }

NonTerminal<Concept> ConceptNegationImpl::get_concept() const { return m_concept; }

/**
 * ConceptValueRestriction
 */

ConceptValueRestrictionImpl::ConceptValueRestrictionImpl(Index index, NonTerminal<Role> role, NonTerminal<Concept> concept_) :
    m_index(index),
    m_role(role),
    m_concept(concept_)
{
}

bool ConceptValueRestrictionImpl::test_match(dl::Constructor<Concept> constructor, const Grammar& grammar) const
{
    auto visitor = ConceptValueRestrictionGrammarVisitor(this, grammar);
    constructor->accept(visitor);
    return visitor.get_result();
}

void ConceptValueRestrictionImpl::accept(Visitor& visitor) const { visitor.visit(this); }

Index ConceptValueRestrictionImpl::get_index() const { return m_index; }

NonTerminal<Role> ConceptValueRestrictionImpl::get_role() const { return m_role; }

NonTerminal<Concept> ConceptValueRestrictionImpl::get_concept() const { return m_concept; }

/**
 * ConceptExistentialQuantification
 */

ConceptExistentialQuantificationImpl::ConceptExistentialQuantificationImpl(Index index,

                                                                           NonTerminal<Role> role,
                                                                           NonTerminal<Concept> concept_) :
    m_index(index),
    m_role(role),
    m_concept(concept_)
{
}

bool ConceptExistentialQuantificationImpl::test_match(dl::Constructor<Concept> constructor, const Grammar& grammar) const
{
    auto visitor = ConceptExistentialQuantificationGrammarVisitor(this, grammar);
    constructor->accept(visitor);
    return visitor.get_result();
}

void ConceptExistentialQuantificationImpl::accept(Visitor& visitor) const { visitor.visit(this); }

Index ConceptExistentialQuantificationImpl::get_index() const { return m_index; }

NonTerminal<Role> ConceptExistentialQuantificationImpl::get_role() const { return m_role; }

NonTerminal<Concept> ConceptExistentialQuantificationImpl::get_concept() const { return m_concept; }

/**
 * ConceptRoleValueMapContainment
 */

ConceptRoleValueMapContainmentImpl::ConceptRoleValueMapContainmentImpl(Index index,

                                                                       NonTerminal<Role> left_role,
                                                                       NonTerminal<Role> right_role) :
    m_index(index),
    m_left_role(left_role),
    m_right_role(right_role)
{
}

bool ConceptRoleValueMapContainmentImpl::test_match(dl::Constructor<Concept> constructor, const Grammar& grammar) const
{
    auto visitor = ConceptRoleValueMapContainmentGrammarVisitor(this, grammar);
    constructor->accept(visitor);
    return visitor.get_result();
}

void ConceptRoleValueMapContainmentImpl::accept(Visitor& visitor) const { visitor.visit(this); }

Index ConceptRoleValueMapContainmentImpl::get_index() const { return m_index; }

NonTerminal<Role> ConceptRoleValueMapContainmentImpl::get_left_role() const { return m_left_role; }

NonTerminal<Role> ConceptRoleValueMapContainmentImpl::get_right_role() const { return m_right_role; }

/**
 * ConceptRoleValueMapEquality
 */

ConceptRoleValueMapEqualityImpl::ConceptRoleValueMapEqualityImpl(Index index,

                                                                 NonTerminal<Role> left_role,
                                                                 NonTerminal<Role> right_role) :
    m_index(index),
    m_left_role(left_role),
    m_right_role(right_role)
{
}

bool ConceptRoleValueMapEqualityImpl::test_match(dl::Constructor<Concept> constructor, const Grammar& grammar) const
{
    auto visitor = ConceptRoleValueMapEqualityGrammarVisitor(this, grammar);
    constructor->accept(visitor);
    return visitor.get_result();
}

void ConceptRoleValueMapEqualityImpl::accept(Visitor& visitor) const { visitor.visit(this); }

Index ConceptRoleValueMapEqualityImpl::get_index() const { return m_index; }

NonTerminal<Role> ConceptRoleValueMapEqualityImpl::get_left_role() const { return m_left_role; }

NonTerminal<Role> ConceptRoleValueMapEqualityImpl::get_right_role() const { return m_right_role; }

/**
 * ConceptNominal
 */

ConceptNominalImpl::ConceptNominalImpl(Index index, Object object) : m_index(index), m_object(object) {}

bool ConceptNominalImpl::test_match(dl::Constructor<Concept> constructor, const Grammar& grammar) const
{
    auto visitor = ConceptNominalGrammarVisitor(this, grammar);
    constructor->accept(visitor);
    return visitor.get_result();
}

void ConceptNominalImpl::accept(Visitor& visitor) const { visitor.visit(this); }

Index ConceptNominalImpl::get_index() const { return m_index; }

Object ConceptNominalImpl::get_object() const { return m_object; }

/**
 * RoleUniversal
 */

RoleUniversalImpl::RoleUniversalImpl(Index index) : m_index(index) {}

bool RoleUniversalImpl::test_match(dl::Constructor<Role> constructor, const Grammar& grammar) const
{
    auto visitor = RoleUniversalGrammarVisitor(this, grammar);
    constructor->accept(visitor);
    return visitor.get_result();
}

void RoleUniversalImpl::accept(Visitor& visitor) const { visitor.visit(this); }

Index RoleUniversalImpl::get_index() const { return m_index; }

/**
 * RoleAtomicState
 */

template<StaticOrFluentOrDerived P>
RoleAtomicStateImpl<P>::RoleAtomicStateImpl(Index index, Predicate<P> predicate) : m_index(index), m_predicate(predicate)
{
}

template<StaticOrFluentOrDerived P>
bool RoleAtomicStateImpl<P>::test_match(dl::Constructor<Role> constructor, const Grammar& grammar) const
{
    auto visitor = RoleAtomicStateGrammarVisitor<P>(this, grammar);
    constructor->accept(visitor);
    return visitor.get_result();
}

template<StaticOrFluentOrDerived P>
void RoleAtomicStateImpl<P>::accept(Visitor& visitor) const
{
    visitor.visit(this);
}

template<StaticOrFluentOrDerived P>
Index RoleAtomicStateImpl<P>::get_index() const
{
    return m_index;
}

template<StaticOrFluentOrDerived P>
Predicate<P> RoleAtomicStateImpl<P>::get_predicate() const
{
    return m_predicate;
}

template class RoleAtomicStateImpl<Static>;
template class RoleAtomicStateImpl<Fluent>;
template class RoleAtomicStateImpl<Derived>;

/**
 * RoleAtomicGoal
 */

template<StaticOrFluentOrDerived P>
RoleAtomicGoalImpl<P>::RoleAtomicGoalImpl(Index index, Predicate<P> predicate, bool is_negated) :
    m_index(index),

    m_predicate(predicate),
    m_is_negated(is_negated)
{
}

template<StaticOrFluentOrDerived P>
bool RoleAtomicGoalImpl<P>::test_match(dl::Constructor<Role> constructor, const Grammar& grammar) const
{
    auto visitor = RoleAtomicGoalGrammarVisitor<P>(this, grammar);
    constructor->accept(visitor);
    return visitor.get_result();
}

template<StaticOrFluentOrDerived P>
void RoleAtomicGoalImpl<P>::accept(Visitor& visitor) const
{
    visitor.visit(this);
}

template<StaticOrFluentOrDerived P>
Index RoleAtomicGoalImpl<P>::get_index() const
{
    return m_index;
}

template<StaticOrFluentOrDerived P>
Predicate<P> RoleAtomicGoalImpl<P>::get_predicate() const
{
    return m_predicate;
}

template<StaticOrFluentOrDerived P>
bool RoleAtomicGoalImpl<P>::is_negated() const
{
    return m_is_negated;
}

template class RoleAtomicGoalImpl<Static>;
template class RoleAtomicGoalImpl<Fluent>;
template class RoleAtomicGoalImpl<Derived>;

/**
 * RoleIntersection
 */

RoleIntersectionImpl::RoleIntersectionImpl(Index index, NonTerminal<Role> left_role, NonTerminal<Role> right_role) :
    m_index(index),

    m_left_role(left_role),
    m_right_role(right_role)
{
}

bool RoleIntersectionImpl::test_match(dl::Constructor<Role> constructor, const Grammar& grammar) const
{
    auto visitor = RoleIntersectionGrammarVisitor(this, grammar);
    constructor->accept(visitor);
    return visitor.get_result();
}

void RoleIntersectionImpl::accept(Visitor& visitor) const { visitor.visit(this); }

Index RoleIntersectionImpl::get_index() const { return m_index; }

NonTerminal<Role> RoleIntersectionImpl::get_left_role() const { return m_left_role; }

NonTerminal<Role> RoleIntersectionImpl::get_right_role() const { return m_right_role; }

/**
 * RoleUnion
 */

RoleUnionImpl::RoleUnionImpl(Index index, NonTerminal<Role> left_role, NonTerminal<Role> right_role) :
    m_index(index),

    m_left_role(left_role),
    m_right_role(right_role)
{
}

bool RoleUnionImpl::test_match(dl::Constructor<Role> constructor, const Grammar& grammar) const
{
    auto visitor = RoleUnionGrammarVisitor(this, grammar);
    constructor->accept(visitor);
    return visitor.get_result();
}

void RoleUnionImpl::accept(Visitor& visitor) const { visitor.visit(this); }

Index RoleUnionImpl::get_index() const { return m_index; }

NonTerminal<Role> RoleUnionImpl::get_left_role() const { return m_left_role; }

NonTerminal<Role> RoleUnionImpl::get_right_role() const { return m_right_role; }

/**
 * RoleComplement
 */

RoleComplementImpl::RoleComplementImpl(Index index, NonTerminal<Role> role) : m_index(index), m_role(role) {}

bool RoleComplementImpl::test_match(dl::Constructor<Role> constructor, const Grammar& grammar) const
{
    auto visitor = RoleComplementGrammarVisitor(this, grammar);
    constructor->accept(visitor);
    return visitor.get_result();
}

void RoleComplementImpl::accept(Visitor& visitor) const { visitor.visit(this); }

Index RoleComplementImpl::get_index() const { return m_index; }

NonTerminal<Role> RoleComplementImpl::get_role() const { return m_role; }

/**
 * RoleInverse
 */

RoleInverseImpl::RoleInverseImpl(Index index, NonTerminal<Role> role) : m_index(index), m_role(role) {}

bool RoleInverseImpl::test_match(dl::Constructor<Role> constructor, const Grammar& grammar) const
{
    auto visitor = RoleInverseGrammarVisitor(this, grammar);
    constructor->accept(visitor);
    return visitor.get_result();
}

void RoleInverseImpl::accept(Visitor& visitor) const { visitor.visit(this); }

Index RoleInverseImpl::get_index() const { return m_index; }

NonTerminal<Role> RoleInverseImpl::get_role() const { return m_role; }

/**
 * RoleComposition
 */

RoleCompositionImpl::RoleCompositionImpl(Index index, NonTerminal<Role> left_role, NonTerminal<Role> right_role) :
    m_index(index),

    m_left_role(left_role),
    m_right_role(right_role)
{
}

bool RoleCompositionImpl::test_match(dl::Constructor<Role> constructor, const Grammar& grammar) const
{
    auto visitor = RoleCompositionGrammarVisitor(this, grammar);
    constructor->accept(visitor);
    return visitor.get_result();
}

void RoleCompositionImpl::accept(Visitor& visitor) const { visitor.visit(this); }

Index RoleCompositionImpl::get_index() const { return m_index; }

NonTerminal<Role> RoleCompositionImpl::get_left_role() const { return m_left_role; }

NonTerminal<Role> RoleCompositionImpl::get_right_role() const { return m_right_role; }

/**
 * RoleTransitiveClosure
 */

RoleTransitiveClosureImpl::RoleTransitiveClosureImpl(Index index, NonTerminal<Role> role) : m_index(index), m_role(role) {}

bool RoleTransitiveClosureImpl::test_match(dl::Constructor<Role> constructor, const Grammar& grammar) const
{
    auto visitor = RoleTransitiveClosureGrammarVisitor(this, grammar);
    constructor->accept(visitor);
    return visitor.get_result();
}

void RoleTransitiveClosureImpl::accept(Visitor& visitor) const { visitor.visit(this); }

Index RoleTransitiveClosureImpl::get_index() const { return m_index; }

NonTerminal<Role> RoleTransitiveClosureImpl::get_role() const { return m_role; }

/**
 * RoleReflexiveTransitiveClosure
 */

RoleReflexiveTransitiveClosureImpl::RoleReflexiveTransitiveClosureImpl(Index index, NonTerminal<Role> role) :
    m_index(index),

    m_role(role)
{
}

bool RoleReflexiveTransitiveClosureImpl::test_match(dl::Constructor<Role> constructor, const Grammar& grammar) const
{
    auto visitor = RoleReflexiveTransitiveClosureGrammarVisitor(this, grammar);
    constructor->accept(visitor);
    return visitor.get_result();
}

void RoleReflexiveTransitiveClosureImpl::accept(Visitor& visitor) const { visitor.visit(this); }

Index RoleReflexiveTransitiveClosureImpl::get_index() const { return m_index; }

NonTerminal<Role> RoleReflexiveTransitiveClosureImpl::get_role() const { return m_role; }

/**
 * RoleRestriction
 */

RoleRestrictionImpl::RoleRestrictionImpl(Index index, NonTerminal<Role> role, NonTerminal<Concept> concept_) :
    m_index(index),

    m_role(role),
    m_concept(concept_)
{
}

bool RoleRestrictionImpl::test_match(dl::Constructor<Role> constructor, const Grammar& grammar) const
{
    auto visitor = RoleRestrictionGrammarVisitor(this, grammar);
    constructor->accept(visitor);
    return visitor.get_result();
}

void RoleRestrictionImpl::accept(Visitor& visitor) const { visitor.visit(this); }

Index RoleRestrictionImpl::get_index() const { return m_index; }

NonTerminal<Role> RoleRestrictionImpl::get_role() const { return m_role; }

NonTerminal<Concept> RoleRestrictionImpl::get_concept() const { return m_concept; }

/**
 * RoleIdentity
 */

RoleIdentityImpl::RoleIdentityImpl(Index index, NonTerminal<Concept> concept_) : m_index(index), m_concept(concept_) {}

bool RoleIdentityImpl::test_match(dl::Constructor<Role> constructor, const Grammar& grammar) const
{
    auto visitor = RoleIdentityGrammarVisitor(this, grammar);
    constructor->accept(visitor);
    return visitor.get_result();
}

void RoleIdentityImpl::accept(Visitor& visitor) const { visitor.visit(this); }

Index RoleIdentityImpl::get_index() const { return m_index; }

NonTerminal<Concept> RoleIdentityImpl::get_concept() const { return m_concept; }

/**
 * Booleans
 */

template<StaticOrFluentOrDerived P>
BooleanAtomicStateImpl<P>::BooleanAtomicStateImpl(Index index, Predicate<P> predicate) : m_index(index), m_predicate(predicate)
{
}

template<StaticOrFluentOrDerived P>
bool BooleanAtomicStateImpl<P>::test_match(dl::Constructor<Boolean> constructor, const Grammar& grammar) const
{
    auto visitor = BooleanAtomicStateGrammarVisitor<P>(this, grammar);
    constructor->accept(visitor);
    return visitor.get_result();
}

template<StaticOrFluentOrDerived P>
void BooleanAtomicStateImpl<P>::accept(Visitor& visitor) const
{
    visitor.visit(this);
}

template<StaticOrFluentOrDerived P>
Index BooleanAtomicStateImpl<P>::get_index() const
{
    return m_index;
}

template<StaticOrFluentOrDerived P>
Predicate<P> BooleanAtomicStateImpl<P>::get_predicate() const
{
    return m_predicate;
}

template class BooleanAtomicStateImpl<Static>;
template class BooleanAtomicStateImpl<Fluent>;
template class BooleanAtomicStateImpl<Derived>;

template<DescriptionLogicCategory D>
BooleanNonemptyImpl<D>::BooleanNonemptyImpl(Index index, NonTerminal<D> nonterminal) : m_index(index), m_nonterminal(nonterminal)
{
}

template<DescriptionLogicCategory D>
bool BooleanNonemptyImpl<D>::test_match(dl::Constructor<Boolean> constructor, const Grammar& grammar) const
{
    auto visitor = BooleanNonemptyGrammarVisitor<D>(this, grammar);
    constructor->accept(visitor);
    return visitor.get_result();
}

template<DescriptionLogicCategory D>
void BooleanNonemptyImpl<D>::accept(Visitor& visitor) const
{
    visitor.visit(this);
}

template<DescriptionLogicCategory D>
Index BooleanNonemptyImpl<D>::get_index() const
{
    return m_index;
}

template<DescriptionLogicCategory D>
NonTerminal<D> BooleanNonemptyImpl<D>::get_nonterminal() const
{
    return m_nonterminal;
}

template class BooleanNonemptyImpl<Concept>;
template class BooleanNonemptyImpl<Role>;

/**
 * Numericals
 */

template<DescriptionLogicCategory D>
NumericalCountImpl<D>::NumericalCountImpl(Index index, NonTerminal<D> nonterminal) : m_index(index), m_nonterminal(nonterminal)
{
}

template<DescriptionLogicCategory D>
bool NumericalCountImpl<D>::test_match(dl::Constructor<Numerical> constructor, const Grammar& grammar) const
{
    auto visitor = NumericalCountGrammarVisitor<D>(this, grammar);
    constructor->accept(visitor);
    return visitor.get_result();
}

template<DescriptionLogicCategory D>
void NumericalCountImpl<D>::accept(Visitor& visitor) const
{
    visitor.visit(this);
}

template<DescriptionLogicCategory D>
Index NumericalCountImpl<D>::get_index() const
{
    return m_index;
}

template<DescriptionLogicCategory D>
NonTerminal<D> NumericalCountImpl<D>::get_nonterminal() const
{
    return m_nonterminal;
}

template class NumericalCountImpl<Concept>;
template class NumericalCountImpl<Role>;

NumericalDistanceImpl::NumericalDistanceImpl(Index index, NonTerminal<Concept> left_concept, NonTerminal<Role> role, NonTerminal<Concept> right_concept) :
    m_index(index),
    m_left_concept(left_concept),
    m_role(role),
    m_right_concept(right_concept)
{
}

bool NumericalDistanceImpl::test_match(dl::Constructor<Numerical> constructor, const Grammar& grammar) const
{
    auto visitor = NumericalDistanceGrammarVisitor(this, grammar);
    constructor->accept(visitor);
    return visitor.get_result();
}

void NumericalDistanceImpl::accept(Visitor& visitor) const { visitor.visit(this); }

Index NumericalDistanceImpl::get_index() const { return m_index; }

NonTerminal<Concept> NumericalDistanceImpl::get_left_concept() const { return m_left_concept; }

NonTerminal<Role> NumericalDistanceImpl::get_role() const { return m_role; }

NonTerminal<Concept> NumericalDistanceImpl::get_right_concept() const { return m_right_concept; }
}
