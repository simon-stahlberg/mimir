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

#include "mimir/languages/description_logics/grammar_constructors.hpp"

#include "mimir/languages/description_logics/constructor_visitor_grammar.hpp"
#include "mimir/languages/description_logics/grammar.hpp"
#include "mimir/languages/description_logics/grammar_visitor_interface.hpp"

using namespace mimir::formalism;

namespace mimir::languages::dl::grammar
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

    if (!nonterminal_to_derivation_rules.contains(this))
    {
        return false;
    }

    const auto& rules = nonterminal_to_derivation_rules.at(this);

    return std::any_of(rules.begin(), rules.end(), [&, constructor](auto&& rule) { return rule->test_match(constructor, grammar); });
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
 * ConstructorOrNonTerminal
 */

template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
ConstructorOrNonTerminalImpl<D>::ConstructorOrNonTerminalImpl(Index index, std::variant<Constructor<D>, NonTerminal<D>> choice) :
    m_index(index),
    m_choice(std::move(choice))
{
}

template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
bool ConstructorOrNonTerminalImpl<D>::test_match(dl::Constructor<D> constructor, const Grammar& grammar) const
{
    return std::visit([&, constructor](const auto& arg) -> bool { return arg->test_match(constructor, grammar); }, m_choice);
}

template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
void ConstructorOrNonTerminalImpl<D>::accept(IVisitor& visitor) const
{
    visitor.visit(this);
}

template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
Index ConstructorOrNonTerminalImpl<D>::get_index() const
{
    return m_index;
}

template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
const std::variant<Constructor<D>, NonTerminal<D>>& ConstructorOrNonTerminalImpl<D>::get_constructor_or_non_terminal() const
{
    return m_choice;
}

template class ConstructorOrNonTerminalImpl<ConceptTag>;
template class ConstructorOrNonTerminalImpl<RoleTag>;
template class ConstructorOrNonTerminalImpl<BooleanTag>;
template class ConstructorOrNonTerminalImpl<NumericalTag>;

/**
 * DerivationRule
 */

template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
DerivationRuleImpl<D>::DerivationRuleImpl(Index index, NonTerminal<D> non_terminal, ConstructorOrNonTerminalList<D> constructor_or_non_terminals) :
    m_index(index),
    m_non_terminal(non_terminal),
    m_constructor_or_non_terminals(std::move(constructor_or_non_terminals))
{
    /* Canonize */
    std::sort(m_constructor_or_non_terminals.begin(),
              m_constructor_or_non_terminals.end(),
              [](const auto& lhs, const auto& rhs) { return lhs->get_index() < rhs->get_index(); });
}

template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
bool DerivationRuleImpl<D>::test_match(dl::Constructor<D> constructor, const Grammar& grammar) const
{
    return std::any_of(m_constructor_or_non_terminals.begin(),
                       m_constructor_or_non_terminals.end(),
                       [&, constructor](const auto& choice) { return choice->test_match(constructor, grammar); });
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
NonTerminal<D> DerivationRuleImpl<D>::get_non_terminal() const
{
    return m_non_terminal;
}

template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
const ConstructorOrNonTerminalList<D>& DerivationRuleImpl<D>::get_constructor_or_non_terminals() const
{
    return m_constructor_or_non_terminals;
}

template class DerivationRuleImpl<ConceptTag>;
template class DerivationRuleImpl<RoleTag>;
template class DerivationRuleImpl<BooleanTag>;
template class DerivationRuleImpl<NumericalTag>;

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
ConceptIntersectionImpl::ConceptIntersectionImpl(Index index,
                                                 ConstructorOrNonTerminal<ConceptTag> left_concept_or_non_terminal,
                                                 ConstructorOrNonTerminal<ConceptTag> right_concept_or_non_terminal) :
    m_index(index),
    m_left_concept_or_non_terminal(left_concept_or_non_terminal),
    m_right_concept_or_non_terminal(right_concept_or_non_terminal)
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

ConstructorOrNonTerminal<ConceptTag> ConceptIntersectionImpl::get_left_concept_or_non_terminal() const { return m_left_concept_or_non_terminal; }

ConstructorOrNonTerminal<ConceptTag> ConceptIntersectionImpl::get_right_concept_or_non_terminal() const { return m_right_concept_or_non_terminal; }

/**
 * ConceptUnion
 */

ConceptUnionImpl::ConceptUnionImpl(Index index,
                                   ConstructorOrNonTerminal<ConceptTag> left_concept_or_non_terminal,
                                   ConstructorOrNonTerminal<ConceptTag> right_concept_or_non_terminal) :
    m_index(index),
    m_left_concept_or_non_terminal(left_concept_or_non_terminal),
    m_right_concept_or_non_terminal(right_concept_or_non_terminal)
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

ConstructorOrNonTerminal<ConceptTag> ConceptUnionImpl::get_left_concept_or_non_terminal() const { return m_left_concept_or_non_terminal; }

ConstructorOrNonTerminal<ConceptTag> ConceptUnionImpl::get_right_concept_or_non_terminal() const { return m_right_concept_or_non_terminal; }

/**
 * ConceptNegation
 */

ConceptNegationImpl::ConceptNegationImpl(Index index, ConstructorOrNonTerminal<ConceptTag> concept_or_non_terminal) :
    m_index(index),
    m_concept_or_non_terminal(concept_or_non_terminal)
{
}

bool ConceptNegationImpl::test_match(dl::Constructor<ConceptTag> constructor, const Grammar& grammar) const
{
    auto visitor = ConceptNegationGrammarVisitor(this, grammar);
    constructor->accept(visitor);
    return visitor.get_result();
}

void ConceptNegationImpl::accept(IVisitor& visitor) const { visitor.visit(this); }

Index ConceptNegationImpl::get_index() const { return m_index; }

ConstructorOrNonTerminal<ConceptTag> ConceptNegationImpl::get_concept_or_non_terminal() const { return m_concept_or_non_terminal; }

/**
 * ConceptValueRestriction
 */

ConceptValueRestrictionImpl::ConceptValueRestrictionImpl(Index index,
                                                         ConstructorOrNonTerminal<RoleTag> role_or_non_terminal,
                                                         ConstructorOrNonTerminal<ConceptTag> concept_or_non_terminal) :
    m_index(index),
    m_role_or_non_terminal(role_or_non_terminal),
    m_concept_or_non_terminal(concept_or_non_terminal)
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

ConstructorOrNonTerminal<RoleTag> ConceptValueRestrictionImpl::get_role_or_non_terminal() const { return m_role_or_non_terminal; }

ConstructorOrNonTerminal<ConceptTag> ConceptValueRestrictionImpl::get_concept_or_non_terminal() const { return m_concept_or_non_terminal; }

/**
 * ConceptExistentialQuantification
 */

ConceptExistentialQuantificationImpl::ConceptExistentialQuantificationImpl(Index index,
                                                                           ConstructorOrNonTerminal<RoleTag> role_or_non_terminal,
                                                                           ConstructorOrNonTerminal<ConceptTag> concept_or_non_terminal) :
    m_index(index),
    m_role_or_non_terminal(role_or_non_terminal),
    m_concept_or_non_terminal(concept_or_non_terminal)
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

ConstructorOrNonTerminal<RoleTag> ConceptExistentialQuantificationImpl::get_role_or_non_terminal() const { return m_role_or_non_terminal; }

ConstructorOrNonTerminal<ConceptTag> ConceptExistentialQuantificationImpl::get_concept_or_non_terminal() const { return m_concept_or_non_terminal; }

/**
 * ConceptRoleValueMapContainment
 */

ConceptRoleValueMapContainmentImpl::ConceptRoleValueMapContainmentImpl(Index index,
                                                                       ConstructorOrNonTerminal<RoleTag> left_role_or_non_terminal,
                                                                       ConstructorOrNonTerminal<RoleTag> right_role_or_non_terminal) :
    m_index(index),
    m_left_role_or_non_terminal(left_role_or_non_terminal),
    m_right_role_or_non_terminal(right_role_or_non_terminal)
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

ConstructorOrNonTerminal<RoleTag> ConceptRoleValueMapContainmentImpl::get_left_role_or_non_terminal() const { return m_left_role_or_non_terminal; }

ConstructorOrNonTerminal<RoleTag> ConceptRoleValueMapContainmentImpl::get_right_role_or_non_terminal() const { return m_right_role_or_non_terminal; }

/**
 * ConceptRoleValueMapEquality
 */

ConceptRoleValueMapEqualityImpl::ConceptRoleValueMapEqualityImpl(Index index,
                                                                 ConstructorOrNonTerminal<RoleTag> left_role_or_non_terminal,
                                                                 ConstructorOrNonTerminal<RoleTag> right_role_or_non_terminal) :
    m_index(index),
    m_left_role_or_non_terminal(left_role_or_non_terminal),
    m_right_role_or_non_terminal(right_role_or_non_terminal)
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

ConstructorOrNonTerminal<RoleTag> ConceptRoleValueMapEqualityImpl::get_left_role_or_non_terminal() const { return m_left_role_or_non_terminal; }

ConstructorOrNonTerminal<RoleTag> ConceptRoleValueMapEqualityImpl::get_right_role_or_non_terminal() const { return m_right_role_or_non_terminal; }

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
RoleAtomicGoalImpl<P>::RoleAtomicGoalImpl(Index index, Predicate<P> predicate, bool polarity) : m_index(index), m_predicate(predicate), m_polarity(polarity)
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

RoleIntersectionImpl::RoleIntersectionImpl(Index index,
                                           ConstructorOrNonTerminal<RoleTag> left_role_or_non_terminal,
                                           ConstructorOrNonTerminal<RoleTag> right_role_or_non_terminal) :
    m_index(index),
    m_left_role_or_non_terminal(left_role_or_non_terminal),
    m_right_role_or_non_terminal(right_role_or_non_terminal)
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

ConstructorOrNonTerminal<RoleTag> RoleIntersectionImpl::get_left_role_or_non_terminal() const { return m_left_role_or_non_terminal; }

ConstructorOrNonTerminal<RoleTag> RoleIntersectionImpl::get_right_role_or_non_terminal() const { return m_right_role_or_non_terminal; }

/**
 * RoleUnion
 */

RoleUnionImpl::RoleUnionImpl(Index index,
                             ConstructorOrNonTerminal<RoleTag> left_role_or_non_terminal,
                             ConstructorOrNonTerminal<RoleTag> right_role_or_non_terminal) :
    m_index(index),
    m_left_role_or_non_terminal(left_role_or_non_terminal),
    m_right_role_or_non_terminal(right_role_or_non_terminal)
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

ConstructorOrNonTerminal<RoleTag> RoleUnionImpl::get_left_role_or_non_terminal() const { return m_left_role_or_non_terminal; }

ConstructorOrNonTerminal<RoleTag> RoleUnionImpl::get_right_role_or_non_terminal() const { return m_right_role_or_non_terminal; }

/**
 * RoleComplement
 */

RoleComplementImpl::RoleComplementImpl(Index index, ConstructorOrNonTerminal<RoleTag> role_or_non_terminal) :
    m_index(index),
    m_role_or_non_terminal(role_or_non_terminal)
{
}

bool RoleComplementImpl::test_match(dl::Constructor<RoleTag> constructor, const Grammar& grammar) const
{
    auto visitor = RoleComplementGrammarVisitor(this, grammar);
    constructor->accept(visitor);
    return visitor.get_result();
}

void RoleComplementImpl::accept(IVisitor& visitor) const { visitor.visit(this); }

Index RoleComplementImpl::get_index() const { return m_index; }

ConstructorOrNonTerminal<RoleTag> RoleComplementImpl::get_role_or_non_terminal() const { return m_role_or_non_terminal; }

/**
 * RoleInverse
 */

RoleInverseImpl::RoleInverseImpl(Index index, ConstructorOrNonTerminal<RoleTag> role_or_non_terminal) :
    m_index(index),
    m_role_or_non_terminal(role_or_non_terminal)
{
}

bool RoleInverseImpl::test_match(dl::Constructor<RoleTag> constructor, const Grammar& grammar) const
{
    auto visitor = RoleInverseGrammarVisitor(this, grammar);
    constructor->accept(visitor);
    return visitor.get_result();
}

void RoleInverseImpl::accept(IVisitor& visitor) const { visitor.visit(this); }

Index RoleInverseImpl::get_index() const { return m_index; }

ConstructorOrNonTerminal<RoleTag> RoleInverseImpl::get_role_or_non_terminal() const { return m_role_or_non_terminal; }

/**
 * RoleComposition
 */

RoleCompositionImpl::RoleCompositionImpl(Index index,
                                         ConstructorOrNonTerminal<RoleTag> left_role_or_non_terminal,
                                         ConstructorOrNonTerminal<RoleTag> right_role_or_non_terminal) :
    m_index(index),
    m_left_role_or_non_terminal(left_role_or_non_terminal),
    m_right_role_or_non_terminal(right_role_or_non_terminal)
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

ConstructorOrNonTerminal<RoleTag> RoleCompositionImpl::get_left_role_or_non_terminal() const { return m_left_role_or_non_terminal; }

ConstructorOrNonTerminal<RoleTag> RoleCompositionImpl::get_right_role_or_non_terminal() const { return m_right_role_or_non_terminal; }

/**
 * RoleTransitiveClosure
 */

RoleTransitiveClosureImpl::RoleTransitiveClosureImpl(Index index, ConstructorOrNonTerminal<RoleTag> role_or_non_terminal) :
    m_index(index),
    m_role_or_non_terminal(role_or_non_terminal)
{
}

bool RoleTransitiveClosureImpl::test_match(dl::Constructor<RoleTag> constructor, const Grammar& grammar) const
{
    auto visitor = RoleTransitiveClosureGrammarVisitor(this, grammar);
    constructor->accept(visitor);
    return visitor.get_result();
}

void RoleTransitiveClosureImpl::accept(IVisitor& visitor) const { visitor.visit(this); }

Index RoleTransitiveClosureImpl::get_index() const { return m_index; }

ConstructorOrNonTerminal<RoleTag> RoleTransitiveClosureImpl::get_role_or_non_terminal() const { return m_role_or_non_terminal; }

/**
 * RoleReflexiveTransitiveClosure
 */

RoleReflexiveTransitiveClosureImpl::RoleReflexiveTransitiveClosureImpl(Index index, ConstructorOrNonTerminal<RoleTag> role_or_non_terminal) :
    m_index(index),
    m_role_or_non_terminal(role_or_non_terminal)
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

ConstructorOrNonTerminal<RoleTag> RoleReflexiveTransitiveClosureImpl::get_role_or_non_terminal() const { return m_role_or_non_terminal; }

/**
 * RoleRestriction
 */

RoleRestrictionImpl::RoleRestrictionImpl(Index index,
                                         ConstructorOrNonTerminal<RoleTag> role_or_non_terminal,
                                         ConstructorOrNonTerminal<ConceptTag> concept_or_non_terminal) :
    m_index(index),
    m_role_or_non_terminal(role_or_non_terminal),
    m_concept_or_non_terminal(concept_or_non_terminal)
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

ConstructorOrNonTerminal<RoleTag> RoleRestrictionImpl::get_role_or_non_terminal() const { return m_role_or_non_terminal; }

ConstructorOrNonTerminal<ConceptTag> RoleRestrictionImpl::get_concept_or_non_terminal() const { return m_concept_or_non_terminal; }

/**
 * RoleIdentity
 */

RoleIdentityImpl::RoleIdentityImpl(Index index, ConstructorOrNonTerminal<ConceptTag> concept_or_non_terminal) :
    m_index(index),
    m_concept_or_non_terminal(concept_or_non_terminal)
{
}

bool RoleIdentityImpl::test_match(dl::Constructor<RoleTag> constructor, const Grammar& grammar) const
{
    auto visitor = RoleIdentityGrammarVisitor(this, grammar);
    constructor->accept(visitor);
    return visitor.get_result();
}

void RoleIdentityImpl::accept(IVisitor& visitor) const { visitor.visit(this); }

Index RoleIdentityImpl::get_index() const { return m_index; }

ConstructorOrNonTerminal<ConceptTag> RoleIdentityImpl::get_concept_or_non_terminal() const { return m_concept_or_non_terminal; }

/**
 * BooleanAtomicState
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

/**
 * BooleanNonempty
 */

template<IsConceptOrRoleTag D>
BooleanNonemptyImpl<D>::BooleanNonemptyImpl(Index index, ConstructorOrNonTerminal<D> constructor_or_nonterminal) :
    m_index(index),
    m_constructor_or_nonterminal(constructor_or_nonterminal)
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
ConstructorOrNonTerminal<D> BooleanNonemptyImpl<D>::get_constructor_or_nonterminal() const
{
    return m_constructor_or_nonterminal;
}

template class BooleanNonemptyImpl<ConceptTag>;
template class BooleanNonemptyImpl<RoleTag>;

/**
 * NumericalCount
 */

template<IsConceptOrRoleTag D>
NumericalCountImpl<D>::NumericalCountImpl(Index index, ConstructorOrNonTerminal<D> constructor_or_nonterminal) :
    m_index(index),
    m_constructor_or_nonterminal(constructor_or_nonterminal)
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
ConstructorOrNonTerminal<D> NumericalCountImpl<D>::get_constructor_or_nonterminal() const
{
    return m_constructor_or_nonterminal;
}

template class NumericalCountImpl<ConceptTag>;
template class NumericalCountImpl<RoleTag>;

/**
 * NumericalDistance
 */

NumericalDistanceImpl::NumericalDistanceImpl(Index index,
                                             ConstructorOrNonTerminal<ConceptTag> left_concept_or_nonterminal,
                                             ConstructorOrNonTerminal<RoleTag> role_or_nonterminal,
                                             ConstructorOrNonTerminal<ConceptTag> right_concept_or_nonterminal) :
    m_index(index),
    m_left_concept_or_nonterminal(left_concept_or_nonterminal),
    m_role_or_nonterminal(role_or_nonterminal),
    m_right_concept_or_nonterminal(right_concept_or_nonterminal)
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

ConstructorOrNonTerminal<ConceptTag> NumericalDistanceImpl::get_left_concept_or_nonterminal() const { return m_left_concept_or_nonterminal; }

ConstructorOrNonTerminal<RoleTag> NumericalDistanceImpl::get_role_or_nonterminal() const { return m_role_or_nonterminal; }

ConstructorOrNonTerminal<ConceptTag> NumericalDistanceImpl::get_right_concept_or_nonterminal() const { return m_right_concept_or_nonterminal; }

}
