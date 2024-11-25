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

#include "mimir/common/hash.hpp"
#include "mimir/languages/description_logics/constructor_visitors_grammar.hpp"
#include "mimir/languages/description_logics/grammar_constructor_visitor_interface.hpp"

namespace mimir::dl::grammar
{

/**
 * NonTerminal
 */

template<dl::ConstructorTag D>
NonTerminalImpl<D>::NonTerminalImpl(Index index, std::string name) : m_index(index), m_name(std::move(name)), m_rule(std::nullopt)
{
}

template<dl::ConstructorTag D>
bool NonTerminalImpl<D>::test_match(dl::Constructor<D> constructor) const
{
    assert(m_rule.has_value());
    return m_rule.value()->test_match(constructor);
}

template<dl::ConstructorTag D>
void NonTerminalImpl<D>::accept(Visitor& visitor) const
{
    visitor.visit(this);
}

template<dl::ConstructorTag D>
Index NonTerminalImpl<D>::get_index() const
{
    return m_index;
}

template<dl::ConstructorTag D>
const std::string& NonTerminalImpl<D>::get_name() const
{
    return m_name;
}

template<dl::ConstructorTag D>
DerivationRule<D> NonTerminalImpl<D>::get_rule() const
{
    assert(m_rule.has_value());
    return m_rule.value();
}

template<dl::ConstructorTag D>
void NonTerminalImpl<D>::set_rule(DerivationRule<D> rule) const
{
    m_rule = rule;
}

template class NonTerminalImpl<Concept>;
template class NonTerminalImpl<Role>;

/**
 * ConstructorOrNonTerminal
 */

template<dl::ConstructorTag D>
ConstructorOrNonTerminalImpl<D>::ConstructorOrNonTerminalImpl(Index index, std::variant<Constructor<D>, NonTerminal<D>> choice) :
    m_index(index),
    m_choice(std::move(choice))
{
}

template<dl::ConstructorTag D>
bool ConstructorOrNonTerminalImpl<D>::test_match(dl::Constructor<D> constructor) const
{
    return std::visit([&constructor](const auto& arg) -> bool { return arg->test_match(constructor); }, m_choice);
}

template<dl::ConstructorTag D>
void ConstructorOrNonTerminalImpl<D>::accept(Visitor& visitor) const
{
    visitor.visit(this);
}

template<dl::ConstructorTag D>
Index ConstructorOrNonTerminalImpl<D>::get_index() const
{
    return m_index;
}

template<dl::ConstructorTag D>
const std::variant<Constructor<D>, NonTerminal<D>>& ConstructorOrNonTerminalImpl<D>::get_constructor_or_non_terminal() const
{
    return m_choice;
}

template class ConstructorOrNonTerminalImpl<Concept>;
template class ConstructorOrNonTerminalImpl<Role>;

/**
 * DerivationRule
 */

template<dl::ConstructorTag D>
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

template<dl::ConstructorTag D>
bool DerivationRuleImpl<D>::test_match(dl::Constructor<D> constructor) const
{
    return std::any_of(m_constructor_or_non_terminals.begin(),
                       m_constructor_or_non_terminals.end(),
                       [&constructor](const auto& choice) { return choice->test_match(constructor); });
}

template<dl::ConstructorTag D>
void DerivationRuleImpl<D>::accept(Visitor& visitor) const
{
    visitor.visit(this);
}

template<dl::ConstructorTag D>
Index DerivationRuleImpl<D>::get_index() const
{
    return m_index;
}

template<dl::ConstructorTag D>
NonTerminal<D> DerivationRuleImpl<D>::get_non_terminal() const
{
    return m_non_terminal;
}

template<dl::ConstructorTag D>
const ConstructorOrNonTerminalList<D>& DerivationRuleImpl<D>::get_constructor_or_non_terminals() const
{
    return m_constructor_or_non_terminals;
}

template class DerivationRuleImpl<Concept>;
template class DerivationRuleImpl<Role>;

/**
 * ConceptBot
 */

ConceptBotImpl::ConceptBotImpl(Index index) : m_index(index) {}

bool ConceptBotImpl::test_match(dl::Constructor<Concept> constructor) const
{
    auto visitor = ConceptBotGrammarVisitor(this);
    constructor->accept(visitor);
    return visitor.get_result();
}

void ConceptBotImpl::accept(Visitor& visitor) const { visitor.visit(this); }

Index ConceptBotImpl::get_index() const { return m_index; }

/**
 * ConceptTop
 */

ConceptTopImpl::ConceptTopImpl(Index index) : m_index(index) {}

bool ConceptTopImpl::test_match(dl::Constructor<Concept> constructor) const
{
    auto visitor = ConceptTopGrammarVisitor(this);
    constructor->accept(visitor);
    return visitor.get_result();
}

void ConceptTopImpl::accept(Visitor& visitor) const { visitor.visit(this); }

Index ConceptTopImpl::get_index() const { return m_index; }

/**
 * ConceptAtomicStateImpl
 */

template<PredicateTag P>
ConceptAtomicStateImpl<P>::ConceptAtomicStateImpl(Index index, Predicate<P> predicate) : m_index(index), m_predicate(predicate)
{
}

template<PredicateTag P>
bool ConceptAtomicStateImpl<P>::test_match(dl::Constructor<Concept> constructor) const
{
    auto visitor = ConceptAtomicStateGrammarVisitor<P>(this);
    constructor->accept(visitor);
    return visitor.get_result();
}

template<PredicateTag P>
void ConceptAtomicStateImpl<P>::accept(Visitor& visitor) const
{
    visitor.visit(this);
}

template<PredicateTag P>
Index ConceptAtomicStateImpl<P>::get_index() const
{
    return m_index;
}

template<PredicateTag P>
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

template<PredicateTag P>
ConceptAtomicGoalImpl<P>::ConceptAtomicGoalImpl(Index index, Predicate<P> predicate, bool is_negated) :
    m_index(index),
    m_predicate(predicate),
    m_is_negated(is_negated)
{
}

template<PredicateTag P>
bool ConceptAtomicGoalImpl<P>::test_match(dl::Constructor<Concept> constructor) const
{
    auto visitor = ConceptAtomicGoalGrammarVisitor<P>(this);
    constructor->accept(visitor);
    return visitor.get_result();
}

template<PredicateTag P>
void ConceptAtomicGoalImpl<P>::accept(Visitor& visitor) const
{
    visitor.visit(this);
}

template<PredicateTag P>
Index ConceptAtomicGoalImpl<P>::get_index() const
{
    return m_index;
}

template<PredicateTag P>
Predicate<P> ConceptAtomicGoalImpl<P>::get_predicate() const
{
    return m_predicate;
}

template<PredicateTag P>
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
ConceptIntersectionImpl::ConceptIntersectionImpl(Index index,
                                                 ConstructorOrNonTerminal<Concept> concept_or_non_terminal_left,
                                                 ConstructorOrNonTerminal<Concept> concept_or_non_terminal_right) :
    m_index(index),
    m_concept_or_non_terminal_left(concept_or_non_terminal_left),
    m_concept_or_non_terminal_right(concept_or_non_terminal_right)
{
}

bool ConceptIntersectionImpl::test_match(dl::Constructor<Concept> constructor) const
{
    auto visitor = ConceptIntersectionGrammarVisitor(this);
    constructor->accept(visitor);
    return visitor.get_result();
}

void ConceptIntersectionImpl::accept(Visitor& visitor) const { visitor.visit(this); }

Index ConceptIntersectionImpl::get_index() const { return m_index; }

ConstructorOrNonTerminal<Concept> ConceptIntersectionImpl::get_concept_or_non_terminal_left() const { return m_concept_or_non_terminal_left; }

ConstructorOrNonTerminal<Concept> ConceptIntersectionImpl::get_concept_or_non_terminal_right() const { return m_concept_or_non_terminal_right; }

/**
 * ConceptUnion
 */

ConceptUnionImpl::ConceptUnionImpl(Index index,
                                   ConstructorOrNonTerminal<Concept> concept_or_non_terminal_left,
                                   ConstructorOrNonTerminal<Concept> concept_or_non_terminal_right) :
    m_index(index),
    m_concept_or_non_terminal_left(concept_or_non_terminal_left),
    m_concept_or_non_terminal_right(concept_or_non_terminal_right)
{
}

bool ConceptUnionImpl::test_match(dl::Constructor<Concept> constructor) const
{
    auto visitor = ConceptUnionGrammarVisitor(this);
    constructor->accept(visitor);
    return visitor.get_result();
}

void ConceptUnionImpl::accept(Visitor& visitor) const { visitor.visit(this); }

Index ConceptUnionImpl::get_index() const { return m_index; }

ConstructorOrNonTerminal<Concept> ConceptUnionImpl::get_concept_or_non_terminal_left() const { return m_concept_or_non_terminal_left; }

ConstructorOrNonTerminal<Concept> ConceptUnionImpl::get_concept_or_non_terminal_right() const { return m_concept_or_non_terminal_right; }

/**
 * ConceptNegation
 */

ConceptNegationImpl::ConceptNegationImpl(Index index, ConstructorOrNonTerminal<Concept> concept_or_non_terminal) :
    m_index(index),
    m_concept_or_non_terminal(concept_or_non_terminal)
{
}

bool ConceptNegationImpl::test_match(dl::Constructor<Concept> constructor) const
{
    auto visitor = ConceptNegationGrammarVisitor(this);
    constructor->accept(visitor);
    return visitor.get_result();
}

void ConceptNegationImpl::accept(Visitor& visitor) const { visitor.visit(this); }

Index ConceptNegationImpl::get_index() const { return m_index; }

ConstructorOrNonTerminal<Concept> ConceptNegationImpl::get_concept_or_non_terminal() const { return m_concept_or_non_terminal; }

/**
 * ConceptValueRestriction
 */

ConceptValueRestrictionImpl::ConceptValueRestrictionImpl(Index index,
                                                         ConstructorOrNonTerminal<Role> role_or_non_terminal,
                                                         ConstructorOrNonTerminal<Concept> concept_or_non_terminal) :
    m_index(index),
    m_role_or_non_terminal(role_or_non_terminal),
    m_concept_or_non_terminal(concept_or_non_terminal)
{
}

bool ConceptValueRestrictionImpl::test_match(dl::Constructor<Concept> constructor) const
{
    auto visitor = ConceptValueRestrictionGrammarVisitor(this);
    constructor->accept(visitor);
    return visitor.get_result();
}

void ConceptValueRestrictionImpl::accept(Visitor& visitor) const { visitor.visit(this); }

Index ConceptValueRestrictionImpl::get_index() const { return m_index; }

ConstructorOrNonTerminal<Role> ConceptValueRestrictionImpl::get_role_or_non_terminal() const { return m_role_or_non_terminal; }

ConstructorOrNonTerminal<Concept> ConceptValueRestrictionImpl::get_concept_or_non_terminal() const { return m_concept_or_non_terminal; }

/**
 * ConceptExistentialQuantification
 */

ConceptExistentialQuantificationImpl::ConceptExistentialQuantificationImpl(Index index,
                                                                           ConstructorOrNonTerminal<Role> role_or_non_terminal,
                                                                           ConstructorOrNonTerminal<Concept> concept_or_non_terminal) :
    m_index(index),
    m_role_or_non_terminal(role_or_non_terminal),
    m_concept_or_non_terminal(concept_or_non_terminal)
{
}

bool ConceptExistentialQuantificationImpl::test_match(dl::Constructor<Concept> constructor) const
{
    auto visitor = ConceptExistentialQuantificationGrammarVisitor(this);
    constructor->accept(visitor);
    return visitor.get_result();
}

void ConceptExistentialQuantificationImpl::accept(Visitor& visitor) const { visitor.visit(this); }

Index ConceptExistentialQuantificationImpl::get_index() const { return m_index; }

ConstructorOrNonTerminal<Role> ConceptExistentialQuantificationImpl::get_role_or_non_terminal() const { return m_role_or_non_terminal; }

ConstructorOrNonTerminal<Concept> ConceptExistentialQuantificationImpl::get_concept_or_non_terminal() const { return m_concept_or_non_terminal; }

/**
 * ConceptRoleValueMapContainment
 */

ConceptRoleValueMapContainmentImpl::ConceptRoleValueMapContainmentImpl(Index index,
                                                                       ConstructorOrNonTerminal<Role> role_or_non_terminal_left,
                                                                       ConstructorOrNonTerminal<Role> role_or_non_terminal_right) :
    m_index(index),
    m_role_or_non_terminal_left(role_or_non_terminal_left),
    m_role_or_non_terminal_right(role_or_non_terminal_right)
{
}

bool ConceptRoleValueMapContainmentImpl::test_match(dl::Constructor<Concept> constructor) const
{
    auto visitor = ConceptRoleValueMapContainmentGrammarVisitor(this);
    constructor->accept(visitor);
    return visitor.get_result();
}

void ConceptRoleValueMapContainmentImpl::accept(Visitor& visitor) const { visitor.visit(this); }

Index ConceptRoleValueMapContainmentImpl::get_index() const { return m_index; }

ConstructorOrNonTerminal<Role> ConceptRoleValueMapContainmentImpl::get_role_or_non_terminal_left() const { return m_role_or_non_terminal_left; }

ConstructorOrNonTerminal<Role> ConceptRoleValueMapContainmentImpl::get_role_or_non_terminal_right() const { return m_role_or_non_terminal_right; }

/**
 * ConceptRoleValueMapEquality
 */

ConceptRoleValueMapEqualityImpl::ConceptRoleValueMapEqualityImpl(Index index,
                                                                 ConstructorOrNonTerminal<Role> role_or_non_terminal_left,
                                                                 ConstructorOrNonTerminal<Role> role_or_non_terminal_right) :
    m_index(index),
    m_role_or_non_terminal_left(role_or_non_terminal_left),
    m_role_or_non_terminal_right(role_or_non_terminal_right)
{
}

bool ConceptRoleValueMapEqualityImpl::test_match(dl::Constructor<Concept> constructor) const
{
    auto visitor = ConceptRoleValueMapEqualityGrammarVisitor(this);
    constructor->accept(visitor);
    return visitor.get_result();
}

void ConceptRoleValueMapEqualityImpl::accept(Visitor& visitor) const { visitor.visit(this); }

Index ConceptRoleValueMapEqualityImpl::get_index() const { return m_index; }

ConstructorOrNonTerminal<Role> ConceptRoleValueMapEqualityImpl::get_role_or_non_terminal_left() const { return m_role_or_non_terminal_left; }

ConstructorOrNonTerminal<Role> ConceptRoleValueMapEqualityImpl::get_role_or_non_terminal_right() const { return m_role_or_non_terminal_right; }

/**
 * ConceptNominal
 */

ConceptNominalImpl::ConceptNominalImpl(Index index, Object object) : m_index(index), m_object(object) {}

bool ConceptNominalImpl::test_match(dl::Constructor<Concept> constructor) const
{
    auto visitor = ConceptNominalGrammarVisitor(this);
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

bool RoleUniversalImpl::test_match(dl::Constructor<Role> constructor) const
{
    auto visitor = RoleUniversalGrammarVisitor(this);
    constructor->accept(visitor);
    return visitor.get_result();
}

void RoleUniversalImpl::accept(Visitor& visitor) const { visitor.visit(this); }

Index RoleUniversalImpl::get_index() const { return m_index; }

/**
 * RoleAtomicState
 */

template<PredicateTag P>
RoleAtomicStateImpl<P>::RoleAtomicStateImpl(Index index, Predicate<P> predicate) : m_index(index), m_predicate(predicate)
{
}

template<PredicateTag P>
bool RoleAtomicStateImpl<P>::test_match(dl::Constructor<Role> constructor) const
{
    auto visitor = RoleAtomicStateGrammarVisitor<P>(this);
    constructor->accept(visitor);
    return visitor.get_result();
}

template<PredicateTag P>
void RoleAtomicStateImpl<P>::accept(Visitor& visitor) const
{
    visitor.visit(this);
}

template<PredicateTag P>
Index RoleAtomicStateImpl<P>::get_index() const
{
    return m_index;
}

template<PredicateTag P>
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

template<PredicateTag P>
RoleAtomicGoalImpl<P>::RoleAtomicGoalImpl(Index index, Predicate<P> predicate, bool is_negated) :
    m_index(index),
    m_predicate(predicate),
    m_is_negated(is_negated)
{
}

template<PredicateTag P>
bool RoleAtomicGoalImpl<P>::test_match(dl::Constructor<Role> constructor) const
{
    auto visitor = RoleAtomicGoalGrammarVisitor<P>(this);
    constructor->accept(visitor);
    return visitor.get_result();
}

template<PredicateTag P>
void RoleAtomicGoalImpl<P>::accept(Visitor& visitor) const
{
    visitor.visit(this);
}

template<PredicateTag P>
Index RoleAtomicGoalImpl<P>::get_index() const
{
    return m_index;
}

template<PredicateTag P>
Predicate<P> RoleAtomicGoalImpl<P>::get_predicate() const
{
    return m_predicate;
}

template<PredicateTag P>
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

RoleIntersectionImpl::RoleIntersectionImpl(Index index,
                                           ConstructorOrNonTerminal<Role> role_or_non_terminal_left,
                                           ConstructorOrNonTerminal<Role> role_or_non_terminal_right) :
    m_index(index),
    m_role_or_non_terminal_left(role_or_non_terminal_left),
    m_role_or_non_terminal_right(role_or_non_terminal_right)
{
}

bool RoleIntersectionImpl::test_match(dl::Constructor<Role> constructor) const
{
    auto visitor = RoleIntersectionGrammarVisitor(this);
    constructor->accept(visitor);
    return visitor.get_result();
}

void RoleIntersectionImpl::accept(Visitor& visitor) const { visitor.visit(this); }

Index RoleIntersectionImpl::get_index() const { return m_index; }

ConstructorOrNonTerminal<Role> RoleIntersectionImpl::get_role_or_non_terminal_left() const { return m_role_or_non_terminal_left; }

ConstructorOrNonTerminal<Role> RoleIntersectionImpl::get_role_or_non_terminal_right() const { return m_role_or_non_terminal_right; }

/**
 * RoleUnion
 */

RoleUnionImpl::RoleUnionImpl(Index index, ConstructorOrNonTerminal<Role> role_or_non_terminal_left, ConstructorOrNonTerminal<Role> role_or_non_terminal_right) :
    m_index(index),
    m_role_or_non_terminal_left(role_or_non_terminal_left),
    m_role_or_non_terminal_right(role_or_non_terminal_right)
{
}

bool RoleUnionImpl::test_match(dl::Constructor<Role> constructor) const
{
    auto visitor = RoleUnionGrammarVisitor(this);
    constructor->accept(visitor);
    return visitor.get_result();
}

void RoleUnionImpl::accept(Visitor& visitor) const { visitor.visit(this); }

Index RoleUnionImpl::get_index() const { return m_index; }

ConstructorOrNonTerminal<Role> RoleUnionImpl::get_role_or_non_terminal_left() const { return m_role_or_non_terminal_left; }

ConstructorOrNonTerminal<Role> RoleUnionImpl::get_role_or_non_terminal_right() const { return m_role_or_non_terminal_right; }

/**
 * RoleComplement
 */

RoleComplementImpl::RoleComplementImpl(Index index, ConstructorOrNonTerminal<Role> role_or_non_terminal) :
    m_index(index),
    m_role_or_non_terminal(role_or_non_terminal)
{
}

bool RoleComplementImpl::test_match(dl::Constructor<Role> constructor) const
{
    auto visitor = RoleComplementGrammarVisitor(this);
    constructor->accept(visitor);
    return visitor.get_result();
}

void RoleComplementImpl::accept(Visitor& visitor) const { visitor.visit(this); }

Index RoleComplementImpl::get_index() const { return m_index; }

ConstructorOrNonTerminal<Role> RoleComplementImpl::get_role_or_non_terminal() const { return m_role_or_non_terminal; }

/**
 * RoleInverse
 */

RoleInverseImpl::RoleInverseImpl(Index index, ConstructorOrNonTerminal<Role> role_or_non_terminal) :
    m_index(index),
    m_role_or_non_terminal(role_or_non_terminal)
{
}

bool RoleInverseImpl::test_match(dl::Constructor<Role> constructor) const
{
    auto visitor = RoleInverseGrammarVisitor(this);
    constructor->accept(visitor);
    return visitor.get_result();
}

void RoleInverseImpl::accept(Visitor& visitor) const { visitor.visit(this); }

Index RoleInverseImpl::get_index() const { return m_index; }

ConstructorOrNonTerminal<Role> RoleInverseImpl::get_role_or_non_terminal() const { return m_role_or_non_terminal; }

/**
 * RoleComposition
 */

RoleCompositionImpl::RoleCompositionImpl(Index index,
                                         ConstructorOrNonTerminal<Role> role_or_non_terminal_left,
                                         ConstructorOrNonTerminal<Role> role_or_non_terminal_right) :
    m_index(index),
    m_role_or_non_terminal_left(role_or_non_terminal_left),
    m_role_or_non_terminal_right(role_or_non_terminal_right)
{
}

bool RoleCompositionImpl::test_match(dl::Constructor<Role> constructor) const
{
    auto visitor = RoleCompositionGrammarVisitor(this);
    constructor->accept(visitor);
    return visitor.get_result();
}

void RoleCompositionImpl::accept(Visitor& visitor) const { visitor.visit(this); }

Index RoleCompositionImpl::get_index() const { return m_index; }

ConstructorOrNonTerminal<Role> RoleCompositionImpl::get_role_or_non_terminal_left() const { return m_role_or_non_terminal_left; }

ConstructorOrNonTerminal<Role> RoleCompositionImpl::get_role_or_non_terminal_right() const { return m_role_or_non_terminal_right; }

/**
 * RoleTransitiveClosure
 */

RoleTransitiveClosureImpl::RoleTransitiveClosureImpl(Index index, ConstructorOrNonTerminal<Role> role_or_non_terminal) :
    m_index(index),
    m_role_or_non_terminal(role_or_non_terminal)
{
}

bool RoleTransitiveClosureImpl::test_match(dl::Constructor<Role> constructor) const
{
    auto visitor = RoleTransitiveClosureGrammarVisitor(this);
    constructor->accept(visitor);
    return visitor.get_result();
}

void RoleTransitiveClosureImpl::accept(Visitor& visitor) const { visitor.visit(this); }

Index RoleTransitiveClosureImpl::get_index() const { return m_index; }

ConstructorOrNonTerminal<Role> RoleTransitiveClosureImpl::get_role_or_non_terminal() const { return m_role_or_non_terminal; }

/**
 * RoleReflexiveTransitiveClosure
 */

RoleReflexiveTransitiveClosureImpl::RoleReflexiveTransitiveClosureImpl(Index index, ConstructorOrNonTerminal<Role> role_or_non_terminal) :
    m_index(index),
    m_role_or_non_terminal(role_or_non_terminal)
{
}

bool RoleReflexiveTransitiveClosureImpl::test_match(dl::Constructor<Role> constructor) const
{
    auto visitor = RoleReflexiveTransitiveClosureGrammarVisitor(this);
    constructor->accept(visitor);
    return visitor.get_result();
}

void RoleReflexiveTransitiveClosureImpl::accept(Visitor& visitor) const { visitor.visit(this); }

Index RoleReflexiveTransitiveClosureImpl::get_index() const { return m_index; }

ConstructorOrNonTerminal<Role> RoleReflexiveTransitiveClosureImpl::get_role_or_non_terminal() const { return m_role_or_non_terminal; }

/**
 * RoleRestriction
 */

RoleRestrictionImpl::RoleRestrictionImpl(Index index,
                                         ConstructorOrNonTerminal<Role> role_or_non_terminal,
                                         ConstructorOrNonTerminal<Concept> concept_or_non_terminal) :
    m_index(index),
    m_role_or_non_terminal(role_or_non_terminal),
    m_concept_or_non_terminal(concept_or_non_terminal)
{
}

bool RoleRestrictionImpl::test_match(dl::Constructor<Role> constructor) const
{
    auto visitor = RoleRestrictionGrammarVisitor(this);
    constructor->accept(visitor);
    return visitor.get_result();
}

void RoleRestrictionImpl::accept(Visitor& visitor) const { visitor.visit(this); }

Index RoleRestrictionImpl::get_index() const { return m_index; }

ConstructorOrNonTerminal<Role> RoleRestrictionImpl::get_role_or_non_terminal() const { return m_role_or_non_terminal; }

ConstructorOrNonTerminal<Concept> RoleRestrictionImpl::get_concept_or_non_terminal() const { return m_concept_or_non_terminal; }

/**
 * RoleIdentity
 */

RoleIdentityImpl::RoleIdentityImpl(Index index, ConstructorOrNonTerminal<Concept> concept_or_non_terminal) :
    m_index(index),
    m_concept_or_non_terminal(concept_or_non_terminal)
{
}

bool RoleIdentityImpl::test_match(dl::Constructor<Role> constructor) const
{
    auto visitor = RoleIdentityGrammarVisitor(this);
    constructor->accept(visitor);
    return visitor.get_result();
}

void RoleIdentityImpl::accept(Visitor& visitor) const { visitor.visit(this); }

Index RoleIdentityImpl::get_index() const { return m_index; }

ConstructorOrNonTerminal<Concept> RoleIdentityImpl::get_concept_or_non_terminal() const { return m_concept_or_non_terminal; }
}
