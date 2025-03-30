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
#include "mimir/languages/description_logics/constructor_visitor_cnf_grammar.hpp"

using namespace mimir::formalism;

namespace mimir::languages::dl::cnf_grammar
{

/**
 * NonTerminal
 */

template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
NonTerminalImpl<D>::NonTerminalImpl(Index index, std::string name) : m_index(index), m_name(std::move(name))
{
}

template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
bool NonTerminalImpl<D>::test_match(dl::Constructor<D> constructor, const Grammar& grammar) const
{
    const auto& nonterminal_to_derivation_rules = boost::hana::at_key(grammar.get_nonterminal_to_derivation_rules(), boost::hana::type<D> {});
    const auto& nonterminal_to_substitution_rules = boost::hana::at_key(grammar.get_nonterminal_to_substitution_rules(), boost::hana::type<D> {});

    if (nonterminal_to_derivation_rules.contains(this))
    {
        const auto& derivation_rules = nonterminal_to_derivation_rules.at(this);
        if (std::any_of(derivation_rules.begin(), derivation_rules.end(), [&](auto&& rule) { return rule->test_match(constructor, grammar); }))
        {
            return true;
        }
    }

    if (nonterminal_to_substitution_rules.contains(this))
    {
        const auto& substitution_rules = nonterminal_to_substitution_rules.at(this);
        if (std::any_of(substitution_rules.begin(), substitution_rules.end(), [&](auto&& rule) { return rule->test_match(constructor, grammar); }))
        {
            return true;
        }
    }

    return false;
}

template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
void NonTerminalImpl<D>::accept(IVisitor& visitor) const
{
    visitor.visit(this);
}

template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
Index NonTerminalImpl<D>::get_index() const
{
    return m_index;
}

template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
const std::string& NonTerminalImpl<D>::get_name() const
{
    return m_name;
}

template class NonTerminalImpl<ConceptTag>;
template class NonTerminalImpl<RoleTag>;
template class NonTerminalImpl<BooleanTag>;
template class NonTerminalImpl<NumericalTag>;

/**
 * DerivationRule
 */

template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
DerivationRuleImpl<D>::DerivationRuleImpl(Index index, NonTerminal<D> head, Constructor<D> body) : m_index(index), m_head(head), m_body(body)
{
}

template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
bool DerivationRuleImpl<D>::test_match(dl::Constructor<D> constructor, const Grammar& grammar) const
{
    return m_body->test_match(constructor, grammar);
}

template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
void DerivationRuleImpl<D>::accept(IVisitor& visitor) const
{
    visitor.visit(this);
}

template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
Index DerivationRuleImpl<D>::get_index() const
{
    return m_index;
}

template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
const NonTerminal<D>& DerivationRuleImpl<D>::get_head() const
{
    return m_head;
}

template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
const Constructor<D>& DerivationRuleImpl<D>::get_body() const
{
    return m_body;
}

template class DerivationRuleImpl<ConceptTag>;
template class DerivationRuleImpl<RoleTag>;
template class DerivationRuleImpl<BooleanTag>;
template class DerivationRuleImpl<NumericalTag>;

/**
 * SubstitutionRule
 */

template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
SubstitutionRuleImpl<D>::SubstitutionRuleImpl(Index index, NonTerminal<D> head, NonTerminal<D> body) : m_index(index), m_head(head), m_body(body)
{
}

template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
bool SubstitutionRuleImpl<D>::test_match(dl::Constructor<D> constructor, const Grammar& grammar) const
{
    return m_body->test_match(constructor, grammar);
}

template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
void SubstitutionRuleImpl<D>::accept(IVisitor& visitor) const
{
    visitor.visit(this);
}

template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
Index SubstitutionRuleImpl<D>::get_index() const
{
    return m_index;
}

template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
const NonTerminal<D>& SubstitutionRuleImpl<D>::get_head() const
{
    return m_head;
}

template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
const NonTerminal<D>& SubstitutionRuleImpl<D>::get_body() const
{
    return m_body;
}

template class SubstitutionRuleImpl<ConceptTag>;
template class SubstitutionRuleImpl<RoleTag>;
template class SubstitutionRuleImpl<BooleanTag>;
template class SubstitutionRuleImpl<NumericalTag>;

/**
 * ConceptBot
 */

ConceptBotImpl::ConceptBotImpl(Index index) : m_index(index) {}

bool ConceptBotImpl::test_match(dl::Constructor<ConceptTag> constructor, const Grammar& grammar) const
{
    auto visitor = ConceptBotGrammarVisitor(this, grammar);
    constructor->accept(visitor);
    return visitor.get_result();
}

void ConceptBotImpl::accept(IVisitor& visitor) const { visitor.visit(this); }

Index ConceptBotImpl::get_index() const { return m_index; }

/**
 * ConceptTop
 */

ConceptTopImpl::ConceptTopImpl(Index index) : m_index(index) {}

bool ConceptTopImpl::test_match(dl::Constructor<ConceptTag> constructor, const Grammar& grammar) const
{
    auto visitor = ConceptTopGrammarVisitor(this, grammar);
    constructor->accept(visitor);
    return visitor.get_result();
}

void ConceptTopImpl::accept(IVisitor& visitor) const { visitor.visit(this); }

Index ConceptTopImpl::get_index() const { return m_index; }

/**
 * ConceptAtomicStateImpl
 */

template<IsStaticOrFluentOrDerivedTag P>
ConceptAtomicStateImpl<P>::ConceptAtomicStateImpl(Index index, Predicate<P> predicate) : m_index(index), m_predicate(predicate)
{
}

template<IsStaticOrFluentOrDerivedTag P>
bool ConceptAtomicStateImpl<P>::test_match(dl::Constructor<ConceptTag> constructor, const Grammar& grammar) const
{
    auto visitor = ConceptAtomicStateGrammarVisitor<P>(this, grammar);
    constructor->accept(visitor);
    return visitor.get_result();
}

template<IsStaticOrFluentOrDerivedTag P>
void ConceptAtomicStateImpl<P>::accept(IVisitor& visitor) const
{
    visitor.visit(this);
}

template<IsStaticOrFluentOrDerivedTag P>
Index ConceptAtomicStateImpl<P>::get_index() const
{
    return m_index;
}

template<IsStaticOrFluentOrDerivedTag P>
Predicate<P> ConceptAtomicStateImpl<P>::get_predicate() const
{
    return m_predicate;
}

template class ConceptAtomicStateImpl<StaticTag>;
template class ConceptAtomicStateImpl<FluentTag>;
template class ConceptAtomicStateImpl<DerivedTag>;

/**
 * ConceptAtomicGoal
 */

template<IsStaticOrFluentOrDerivedTag P>
ConceptAtomicGoalImpl<P>::ConceptAtomicGoalImpl(Index index, Predicate<P> predicate, bool polarity) :
    m_index(index),

    m_predicate(predicate),
    m_polarity(polarity)
{
}

template<IsStaticOrFluentOrDerivedTag P>
bool ConceptAtomicGoalImpl<P>::test_match(dl::Constructor<ConceptTag> constructor, const Grammar& grammar) const
{
    auto visitor = ConceptAtomicGoalGrammarVisitor<P>(this, grammar);
    constructor->accept(visitor);
    return visitor.get_result();
}

template<IsStaticOrFluentOrDerivedTag P>
void ConceptAtomicGoalImpl<P>::accept(IVisitor& visitor) const
{
    visitor.visit(this);
}

template<IsStaticOrFluentOrDerivedTag P>
Index ConceptAtomicGoalImpl<P>::get_index() const
{
    return m_index;
}

template<IsStaticOrFluentOrDerivedTag P>
Predicate<P> ConceptAtomicGoalImpl<P>::get_predicate() const
{
    return m_predicate;
}

template<IsStaticOrFluentOrDerivedTag P>
bool ConceptAtomicGoalImpl<P>::get_polarity() const
{
    return m_polarity;
}

template class ConceptAtomicGoalImpl<StaticTag>;
template class ConceptAtomicGoalImpl<FluentTag>;
template class ConceptAtomicGoalImpl<DerivedTag>;

/**
 * ConceptIntersection
 */
ConceptIntersectionImpl::ConceptIntersectionImpl(Index index, NonTerminal<ConceptTag> left_concept, NonTerminal<ConceptTag> right_concept) :
    m_index(index),
    m_left_concept(left_concept),
    m_right_concept(right_concept)
{
}

bool ConceptIntersectionImpl::test_match(dl::Constructor<ConceptTag> constructor, const Grammar& grammar) const
{
    auto visitor = ConceptIntersectionGrammarVisitor(this, grammar);
    constructor->accept(visitor);
    return visitor.get_result();
}

void ConceptIntersectionImpl::accept(IVisitor& visitor) const { visitor.visit(this); }

Index ConceptIntersectionImpl::get_index() const { return m_index; }

NonTerminal<ConceptTag> ConceptIntersectionImpl::get_left_concept() const { return m_left_concept; }

NonTerminal<ConceptTag> ConceptIntersectionImpl::get_right_concept() const { return m_right_concept; }

/**
 * ConceptUnion
 */

ConceptUnionImpl::ConceptUnionImpl(Index index, NonTerminal<ConceptTag> left_concept, NonTerminal<ConceptTag> right_concept) :
    m_index(index),
    m_left_concept(left_concept),
    m_right_concept(right_concept)
{
}

bool ConceptUnionImpl::test_match(dl::Constructor<ConceptTag> constructor, const Grammar& grammar) const
{
    auto visitor = ConceptUnionGrammarVisitor(this, grammar);
    constructor->accept(visitor);
    return visitor.get_result();
}

void ConceptUnionImpl::accept(IVisitor& visitor) const { visitor.visit(this); }

Index ConceptUnionImpl::get_index() const { return m_index; }

NonTerminal<ConceptTag> ConceptUnionImpl::get_left_concept() const { return m_left_concept; }

NonTerminal<ConceptTag> ConceptUnionImpl::get_right_concept() const { return m_right_concept; }

/**
 * ConceptNegation
 */

ConceptNegationImpl::ConceptNegationImpl(Index index, NonTerminal<ConceptTag> concept_) : m_index(index), m_concept(concept_) {}

bool ConceptNegationImpl::test_match(dl::Constructor<ConceptTag> constructor, const Grammar& grammar) const
{
    auto visitor = ConceptNegationGrammarVisitor(this, grammar);
    constructor->accept(visitor);
    return visitor.get_result();
}

void ConceptNegationImpl::accept(IVisitor& visitor) const { visitor.visit(this); }

Index ConceptNegationImpl::get_index() const { return m_index; }

NonTerminal<ConceptTag> ConceptNegationImpl::get_concept() const { return m_concept; }

/**
 * ConceptValueRestriction
 */

ConceptValueRestrictionImpl::ConceptValueRestrictionImpl(Index index, NonTerminal<RoleTag> role, NonTerminal<ConceptTag> concept_) :
    m_index(index),
    m_role(role),
    m_concept(concept_)
{
}

bool ConceptValueRestrictionImpl::test_match(dl::Constructor<ConceptTag> constructor, const Grammar& grammar) const
{
    auto visitor = ConceptValueRestrictionGrammarVisitor(this, grammar);
    constructor->accept(visitor);
    return visitor.get_result();
}

void ConceptValueRestrictionImpl::accept(IVisitor& visitor) const { visitor.visit(this); }

Index ConceptValueRestrictionImpl::get_index() const { return m_index; }

NonTerminal<RoleTag> ConceptValueRestrictionImpl::get_role() const { return m_role; }

NonTerminal<ConceptTag> ConceptValueRestrictionImpl::get_concept() const { return m_concept; }

/**
 * ConceptExistentialQuantification
 */

ConceptExistentialQuantificationImpl::ConceptExistentialQuantificationImpl(Index index,

                                                                           NonTerminal<RoleTag> role,
                                                                           NonTerminal<ConceptTag> concept_) :
    m_index(index),
    m_role(role),
    m_concept(concept_)
{
}

bool ConceptExistentialQuantificationImpl::test_match(dl::Constructor<ConceptTag> constructor, const Grammar& grammar) const
{
    auto visitor = ConceptExistentialQuantificationGrammarVisitor(this, grammar);
    constructor->accept(visitor);
    return visitor.get_result();
}

void ConceptExistentialQuantificationImpl::accept(IVisitor& visitor) const { visitor.visit(this); }

Index ConceptExistentialQuantificationImpl::get_index() const { return m_index; }

NonTerminal<RoleTag> ConceptExistentialQuantificationImpl::get_role() const { return m_role; }

NonTerminal<ConceptTag> ConceptExistentialQuantificationImpl::get_concept() const { return m_concept; }

/**
 * ConceptRoleValueMapContainment
 */

ConceptRoleValueMapContainmentImpl::ConceptRoleValueMapContainmentImpl(Index index,

                                                                       NonTerminal<RoleTag> left_role,
                                                                       NonTerminal<RoleTag> right_role) :
    m_index(index),
    m_left_role(left_role),
    m_right_role(right_role)
{
}

bool ConceptRoleValueMapContainmentImpl::test_match(dl::Constructor<ConceptTag> constructor, const Grammar& grammar) const
{
    auto visitor = ConceptRoleValueMapContainmentGrammarVisitor(this, grammar);
    constructor->accept(visitor);
    return visitor.get_result();
}

void ConceptRoleValueMapContainmentImpl::accept(IVisitor& visitor) const { visitor.visit(this); }

Index ConceptRoleValueMapContainmentImpl::get_index() const { return m_index; }

NonTerminal<RoleTag> ConceptRoleValueMapContainmentImpl::get_left_role() const { return m_left_role; }

NonTerminal<RoleTag> ConceptRoleValueMapContainmentImpl::get_right_role() const { return m_right_role; }

/**
 * ConceptRoleValueMapEquality
 */

ConceptRoleValueMapEqualityImpl::ConceptRoleValueMapEqualityImpl(Index index,

                                                                 NonTerminal<RoleTag> left_role,
                                                                 NonTerminal<RoleTag> right_role) :
    m_index(index),
    m_left_role(left_role),
    m_right_role(right_role)
{
}

bool ConceptRoleValueMapEqualityImpl::test_match(dl::Constructor<ConceptTag> constructor, const Grammar& grammar) const
{
    auto visitor = ConceptRoleValueMapEqualityGrammarVisitor(this, grammar);
    constructor->accept(visitor);
    return visitor.get_result();
}

void ConceptRoleValueMapEqualityImpl::accept(IVisitor& visitor) const { visitor.visit(this); }

Index ConceptRoleValueMapEqualityImpl::get_index() const { return m_index; }

NonTerminal<RoleTag> ConceptRoleValueMapEqualityImpl::get_left_role() const { return m_left_role; }

NonTerminal<RoleTag> ConceptRoleValueMapEqualityImpl::get_right_role() const { return m_right_role; }

/**
 * ConceptNominal
 */

ConceptNominalImpl::ConceptNominalImpl(Index index, Object object) : m_index(index), m_object(object) {}

bool ConceptNominalImpl::test_match(dl::Constructor<ConceptTag> constructor, const Grammar& grammar) const
{
    auto visitor = ConceptNominalGrammarVisitor(this, grammar);
    constructor->accept(visitor);
    return visitor.get_result();
}

void ConceptNominalImpl::accept(IVisitor& visitor) const { visitor.visit(this); }

Index ConceptNominalImpl::get_index() const { return m_index; }

Object ConceptNominalImpl::get_object() const { return m_object; }

/**
 * RoleUniversal
 */

RoleUniversalImpl::RoleUniversalImpl(Index index) : m_index(index) {}

bool RoleUniversalImpl::test_match(dl::Constructor<RoleTag> constructor, const Grammar& grammar) const
{
    auto visitor = RoleUniversalGrammarVisitor(this, grammar);
    constructor->accept(visitor);
    return visitor.get_result();
}

void RoleUniversalImpl::accept(IVisitor& visitor) const { visitor.visit(this); }

Index RoleUniversalImpl::get_index() const { return m_index; }

/**
 * RoleAtomicState
 */

template<IsStaticOrFluentOrDerivedTag P>
RoleAtomicStateImpl<P>::RoleAtomicStateImpl(Index index, Predicate<P> predicate) : m_index(index), m_predicate(predicate)
{
}

template<IsStaticOrFluentOrDerivedTag P>
bool RoleAtomicStateImpl<P>::test_match(dl::Constructor<RoleTag> constructor, const Grammar& grammar) const
{
    auto visitor = RoleAtomicStateGrammarVisitor<P>(this, grammar);
    constructor->accept(visitor);
    return visitor.get_result();
}

template<IsStaticOrFluentOrDerivedTag P>
void RoleAtomicStateImpl<P>::accept(IVisitor& visitor) const
{
    visitor.visit(this);
}

template<IsStaticOrFluentOrDerivedTag P>
Index RoleAtomicStateImpl<P>::get_index() const
{
    return m_index;
}

template<IsStaticOrFluentOrDerivedTag P>
Predicate<P> RoleAtomicStateImpl<P>::get_predicate() const
{
    return m_predicate;
}

template class RoleAtomicStateImpl<StaticTag>;
template class RoleAtomicStateImpl<FluentTag>;
template class RoleAtomicStateImpl<DerivedTag>;

/**
 * RoleAtomicGoal
 */

template<IsStaticOrFluentOrDerivedTag P>
RoleAtomicGoalImpl<P>::RoleAtomicGoalImpl(Index index, Predicate<P> predicate, bool polarity) :
    m_index(index),

    m_predicate(predicate),
    m_polarity(polarity)
{
}

template<IsStaticOrFluentOrDerivedTag P>
bool RoleAtomicGoalImpl<P>::test_match(dl::Constructor<RoleTag> constructor, const Grammar& grammar) const
{
    auto visitor = RoleAtomicGoalGrammarVisitor<P>(this, grammar);
    constructor->accept(visitor);
    return visitor.get_result();
}

template<IsStaticOrFluentOrDerivedTag P>
void RoleAtomicGoalImpl<P>::accept(IVisitor& visitor) const
{
    visitor.visit(this);
}

template<IsStaticOrFluentOrDerivedTag P>
Index RoleAtomicGoalImpl<P>::get_index() const
{
    return m_index;
}

template<IsStaticOrFluentOrDerivedTag P>
Predicate<P> RoleAtomicGoalImpl<P>::get_predicate() const
{
    return m_predicate;
}

template<IsStaticOrFluentOrDerivedTag P>
bool RoleAtomicGoalImpl<P>::get_polarity() const
{
    return m_polarity;
}

template class RoleAtomicGoalImpl<StaticTag>;
template class RoleAtomicGoalImpl<FluentTag>;
template class RoleAtomicGoalImpl<DerivedTag>;

/**
 * RoleIntersection
 */

RoleIntersectionImpl::RoleIntersectionImpl(Index index, NonTerminal<RoleTag> left_role, NonTerminal<RoleTag> right_role) :
    m_index(index),

    m_left_role(left_role),
    m_right_role(right_role)
{
}

bool RoleIntersectionImpl::test_match(dl::Constructor<RoleTag> constructor, const Grammar& grammar) const
{
    auto visitor = RoleIntersectionGrammarVisitor(this, grammar);
    constructor->accept(visitor);
    return visitor.get_result();
}

void RoleIntersectionImpl::accept(IVisitor& visitor) const { visitor.visit(this); }

Index RoleIntersectionImpl::get_index() const { return m_index; }

NonTerminal<RoleTag> RoleIntersectionImpl::get_left_role() const { return m_left_role; }

NonTerminal<RoleTag> RoleIntersectionImpl::get_right_role() const { return m_right_role; }

/**
 * RoleUnion
 */

RoleUnionImpl::RoleUnionImpl(Index index, NonTerminal<RoleTag> left_role, NonTerminal<RoleTag> right_role) :
    m_index(index),

    m_left_role(left_role),
    m_right_role(right_role)
{
}

bool RoleUnionImpl::test_match(dl::Constructor<RoleTag> constructor, const Grammar& grammar) const
{
    auto visitor = RoleUnionGrammarVisitor(this, grammar);
    constructor->accept(visitor);
    return visitor.get_result();
}

void RoleUnionImpl::accept(IVisitor& visitor) const { visitor.visit(this); }

Index RoleUnionImpl::get_index() const { return m_index; }

NonTerminal<RoleTag> RoleUnionImpl::get_left_role() const { return m_left_role; }

NonTerminal<RoleTag> RoleUnionImpl::get_right_role() const { return m_right_role; }

/**
 * RoleComplement
 */

RoleComplementImpl::RoleComplementImpl(Index index, NonTerminal<RoleTag> role) : m_index(index), m_role(role) {}

bool RoleComplementImpl::test_match(dl::Constructor<RoleTag> constructor, const Grammar& grammar) const
{
    auto visitor = RoleComplementGrammarVisitor(this, grammar);
    constructor->accept(visitor);
    return visitor.get_result();
}

void RoleComplementImpl::accept(IVisitor& visitor) const { visitor.visit(this); }

Index RoleComplementImpl::get_index() const { return m_index; }

NonTerminal<RoleTag> RoleComplementImpl::get_role() const { return m_role; }

/**
 * RoleInverse
 */

RoleInverseImpl::RoleInverseImpl(Index index, NonTerminal<RoleTag> role) : m_index(index), m_role(role) {}

bool RoleInverseImpl::test_match(dl::Constructor<RoleTag> constructor, const Grammar& grammar) const
{
    auto visitor = RoleInverseGrammarVisitor(this, grammar);
    constructor->accept(visitor);
    return visitor.get_result();
}

void RoleInverseImpl::accept(IVisitor& visitor) const { visitor.visit(this); }

Index RoleInverseImpl::get_index() const { return m_index; }

NonTerminal<RoleTag> RoleInverseImpl::get_role() const { return m_role; }

/**
 * RoleComposition
 */

RoleCompositionImpl::RoleCompositionImpl(Index index, NonTerminal<RoleTag> left_role, NonTerminal<RoleTag> right_role) :
    m_index(index),

    m_left_role(left_role),
    m_right_role(right_role)
{
}

bool RoleCompositionImpl::test_match(dl::Constructor<RoleTag> constructor, const Grammar& grammar) const
{
    auto visitor = RoleCompositionGrammarVisitor(this, grammar);
    constructor->accept(visitor);
    return visitor.get_result();
}

void RoleCompositionImpl::accept(IVisitor& visitor) const { visitor.visit(this); }

Index RoleCompositionImpl::get_index() const { return m_index; }

NonTerminal<RoleTag> RoleCompositionImpl::get_left_role() const { return m_left_role; }

NonTerminal<RoleTag> RoleCompositionImpl::get_right_role() const { return m_right_role; }

/**
 * RoleTransitiveClosure
 */

RoleTransitiveClosureImpl::RoleTransitiveClosureImpl(Index index, NonTerminal<RoleTag> role) : m_index(index), m_role(role) {}

bool RoleTransitiveClosureImpl::test_match(dl::Constructor<RoleTag> constructor, const Grammar& grammar) const
{
    auto visitor = RoleTransitiveClosureGrammarVisitor(this, grammar);
    constructor->accept(visitor);
    return visitor.get_result();
}

void RoleTransitiveClosureImpl::accept(IVisitor& visitor) const { visitor.visit(this); }

Index RoleTransitiveClosureImpl::get_index() const { return m_index; }

NonTerminal<RoleTag> RoleTransitiveClosureImpl::get_role() const { return m_role; }

/**
 * RoleReflexiveTransitiveClosure
 */

RoleReflexiveTransitiveClosureImpl::RoleReflexiveTransitiveClosureImpl(Index index, NonTerminal<RoleTag> role) :
    m_index(index),

    m_role(role)
{
}

bool RoleReflexiveTransitiveClosureImpl::test_match(dl::Constructor<RoleTag> constructor, const Grammar& grammar) const
{
    auto visitor = RoleReflexiveTransitiveClosureGrammarVisitor(this, grammar);
    constructor->accept(visitor);
    return visitor.get_result();
}

void RoleReflexiveTransitiveClosureImpl::accept(IVisitor& visitor) const { visitor.visit(this); }

Index RoleReflexiveTransitiveClosureImpl::get_index() const { return m_index; }

NonTerminal<RoleTag> RoleReflexiveTransitiveClosureImpl::get_role() const { return m_role; }

/**
 * RoleRestriction
 */

RoleRestrictionImpl::RoleRestrictionImpl(Index index, NonTerminal<RoleTag> role, NonTerminal<ConceptTag> concept_) :
    m_index(index),

    m_role(role),
    m_concept(concept_)
{
}

bool RoleRestrictionImpl::test_match(dl::Constructor<RoleTag> constructor, const Grammar& grammar) const
{
    auto visitor = RoleRestrictionGrammarVisitor(this, grammar);
    constructor->accept(visitor);
    return visitor.get_result();
}

void RoleRestrictionImpl::accept(IVisitor& visitor) const { visitor.visit(this); }

Index RoleRestrictionImpl::get_index() const { return m_index; }

NonTerminal<RoleTag> RoleRestrictionImpl::get_role() const { return m_role; }

NonTerminal<ConceptTag> RoleRestrictionImpl::get_concept() const { return m_concept; }

/**
 * RoleIdentity
 */

RoleIdentityImpl::RoleIdentityImpl(Index index, NonTerminal<ConceptTag> concept_) : m_index(index), m_concept(concept_) {}

bool RoleIdentityImpl::test_match(dl::Constructor<RoleTag> constructor, const Grammar& grammar) const
{
    auto visitor = RoleIdentityGrammarVisitor(this, grammar);
    constructor->accept(visitor);
    return visitor.get_result();
}

void RoleIdentityImpl::accept(IVisitor& visitor) const { visitor.visit(this); }

Index RoleIdentityImpl::get_index() const { return m_index; }

NonTerminal<ConceptTag> RoleIdentityImpl::get_concept() const { return m_concept; }

/**
 * Booleans
 */

template<IsStaticOrFluentOrDerivedTag P>
BooleanAtomicStateImpl<P>::BooleanAtomicStateImpl(Index index, Predicate<P> predicate) : m_index(index), m_predicate(predicate)
{
}

template<IsStaticOrFluentOrDerivedTag P>
bool BooleanAtomicStateImpl<P>::test_match(dl::Constructor<BooleanTag> constructor, const Grammar& grammar) const
{
    auto visitor = BooleanAtomicStateGrammarVisitor<P>(this, grammar);
    constructor->accept(visitor);
    return visitor.get_result();
}

template<IsStaticOrFluentOrDerivedTag P>
void BooleanAtomicStateImpl<P>::accept(IVisitor& visitor) const
{
    visitor.visit(this);
}

template<IsStaticOrFluentOrDerivedTag P>
Index BooleanAtomicStateImpl<P>::get_index() const
{
    return m_index;
}

template<IsStaticOrFluentOrDerivedTag P>
Predicate<P> BooleanAtomicStateImpl<P>::get_predicate() const
{
    return m_predicate;
}

template class BooleanAtomicStateImpl<StaticTag>;
template class BooleanAtomicStateImpl<FluentTag>;
template class BooleanAtomicStateImpl<DerivedTag>;

template<IsConceptOrRoleTag D>
BooleanNonemptyImpl<D>::BooleanNonemptyImpl(Index index, NonTerminal<D> nonterminal) : m_index(index), m_nonterminal(nonterminal)
{
}

template<IsConceptOrRoleTag D>
bool BooleanNonemptyImpl<D>::test_match(dl::Constructor<BooleanTag> constructor, const Grammar& grammar) const
{
    auto visitor = BooleanNonemptyGrammarVisitor<D>(this, grammar);
    constructor->accept(visitor);
    return visitor.get_result();
}

template<IsConceptOrRoleTag D>
void BooleanNonemptyImpl<D>::accept(IVisitor& visitor) const
{
    visitor.visit(this);
}

template<IsConceptOrRoleTag D>
Index BooleanNonemptyImpl<D>::get_index() const
{
    return m_index;
}

template<IsConceptOrRoleTag D>
NonTerminal<D> BooleanNonemptyImpl<D>::get_nonterminal() const
{
    return m_nonterminal;
}

template class BooleanNonemptyImpl<ConceptTag>;
template class BooleanNonemptyImpl<RoleTag>;

/**
 * Numericals
 */

template<IsConceptOrRoleTag D>
NumericalCountImpl<D>::NumericalCountImpl(Index index, NonTerminal<D> nonterminal) : m_index(index), m_nonterminal(nonterminal)
{
}

template<IsConceptOrRoleTag D>
bool NumericalCountImpl<D>::test_match(dl::Constructor<NumericalTag> constructor, const Grammar& grammar) const
{
    auto visitor = NumericalCountGrammarVisitor<D>(this, grammar);
    constructor->accept(visitor);
    return visitor.get_result();
}

template<IsConceptOrRoleTag D>
void NumericalCountImpl<D>::accept(IVisitor& visitor) const
{
    visitor.visit(this);
}

template<IsConceptOrRoleTag D>
Index NumericalCountImpl<D>::get_index() const
{
    return m_index;
}

template<IsConceptOrRoleTag D>
NonTerminal<D> NumericalCountImpl<D>::get_nonterminal() const
{
    return m_nonterminal;
}

template class NumericalCountImpl<ConceptTag>;
template class NumericalCountImpl<RoleTag>;

NumericalDistanceImpl::NumericalDistanceImpl(Index index,
                                             NonTerminal<ConceptTag> left_concept,
                                             NonTerminal<RoleTag> role,
                                             NonTerminal<ConceptTag> right_concept) :
    m_index(index),
    m_left_concept(left_concept),
    m_role(role),
    m_right_concept(right_concept)
{
}

bool NumericalDistanceImpl::test_match(dl::Constructor<NumericalTag> constructor, const Grammar& grammar) const
{
    auto visitor = NumericalDistanceGrammarVisitor(this, grammar);
    constructor->accept(visitor);
    return visitor.get_result();
}

void NumericalDistanceImpl::accept(IVisitor& visitor) const { visitor.visit(this); }

Index NumericalDistanceImpl::get_index() const { return m_index; }

NonTerminal<ConceptTag> NumericalDistanceImpl::get_left_concept() const { return m_left_concept; }

NonTerminal<RoleTag> NumericalDistanceImpl::get_role() const { return m_role; }

NonTerminal<ConceptTag> NumericalDistanceImpl::get_right_concept() const { return m_right_concept; }
}
