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
#include "mimir/languages/description_logics/grammar_visitors.hpp"

namespace mimir::dl::grammar
{

/**
 * NonTerminal
 */

template<dl::IsConceptOrRole D>
NonTerminalImpl<D>::NonTerminalImpl(Index index, std::string name) : m_index(index), m_name(std::move(name)), m_rule(std::nullopt)
{
}

template<dl::IsConceptOrRole D>
bool NonTerminalImpl<D>::test_match(dl::Constructor<D> constructor) const
{
    assert(m_rule.has_value());
    return m_rule.value()->test_match(constructor);
}

template<dl::IsConceptOrRole D>
Index NonTerminalImpl<D>::get_index() const
{
    return m_index;
}

template<dl::IsConceptOrRole D>
const std::string& NonTerminalImpl<D>::get_name() const
{
    return m_name;
}

template<dl::IsConceptOrRole D>
DerivationRule<D> NonTerminalImpl<D>::get_rule() const
{
    assert(m_rule.has_value());
    return m_rule.value();
}

template<dl::IsConceptOrRole D>
void NonTerminalImpl<D>::set_rule(DerivationRule<D> rule) const
{
    m_rule = rule;
}

template class NonTerminalImpl<Concept>;
template class NonTerminalImpl<Role>;

/**
 * Choice
 */

template<dl::IsConceptOrRole D>
ChoiceImpl<D>::ChoiceImpl(Index index, ConstructorOrNonTerminalChoice<D> choice) : m_index(index), m_choice(std::move(choice))
{
}

template<dl::IsConceptOrRole D>
bool ChoiceImpl<D>::test_match(dl::Constructor<D> constructor) const
{
    return std::visit([&constructor](const auto& arg) -> bool { return arg->test_match(constructor); }, m_choice);
}

template<dl::IsConceptOrRole D>
Index ChoiceImpl<D>::get_index() const
{
    return m_index;
}

template<dl::IsConceptOrRole D>
const ConstructorOrNonTerminalChoice<D>& ChoiceImpl<D>::get_choice() const
{
    return m_choice;
}

template class ChoiceImpl<Concept>;
template class ChoiceImpl<Role>;

/**
 * DerivationRule
 */

template<dl::IsConceptOrRole D>
DerivationRuleImpl<D>::DerivationRuleImpl(Index index, ChoiceList<D> choices) : m_index(index), m_choices(std::move(choices))
{
    /* Canonize */
    std::sort(m_choices.begin(), m_choices.end(), [](const auto& lhs, const auto& rhs) { return lhs->get_index() < rhs->get_index(); });
}

template<dl::IsConceptOrRole D>
bool DerivationRuleImpl<D>::test_match(dl::Constructor<D> constructor) const
{
    return std::any_of(m_choices.begin(), m_choices.end(), [&constructor](const auto& choice) { return choice->test_match(constructor); });
}

template<dl::IsConceptOrRole D>
Index DerivationRuleImpl<D>::get_index() const
{
    return m_index;
}

template<dl::IsConceptOrRole D>
const ChoiceList<D>& DerivationRuleImpl<D>::get_choices() const
{
    return m_choices;
}

template class DerivationRuleImpl<Concept>;
template class DerivationRuleImpl<Role>;

/**
 * ConceptPredicateStateImpl
 */

template<PredicateCategory P>
ConceptPredicateStateImpl<P>::ConceptPredicateStateImpl(Index index, Predicate<P> predicate) : m_index(index), m_predicate(predicate)
{
}

template<PredicateCategory P>
bool ConceptPredicateStateImpl<P>::test_match(dl::Constructor<Concept> constructor) const
{
    return constructor->accept(grammar::ConceptPredicateStateVisitor<P>(this));
}

template<PredicateCategory P>
Index ConceptPredicateStateImpl<P>::get_index() const
{
    return m_index;
}

template<PredicateCategory P>
Predicate<P> ConceptPredicateStateImpl<P>::get_predicate() const
{
    return m_predicate;
}

template class ConceptPredicateStateImpl<Static>;
template class ConceptPredicateStateImpl<Fluent>;
template class ConceptPredicateStateImpl<Derived>;

/**
 * ConceptPredicateGoal
 */

template<PredicateCategory P>
ConceptPredicateGoalImpl<P>::ConceptPredicateGoalImpl(Index index, Predicate<P> predicate) : m_index(index), m_predicate(predicate)
{
}

template<PredicateCategory P>
bool ConceptPredicateGoalImpl<P>::test_match(dl::Constructor<Concept> constructor) const
{
    return constructor->accept(ConceptPredicateGoalVisitor<P>(this));
}

template<PredicateCategory P>
Index ConceptPredicateGoalImpl<P>::get_index() const
{
    return m_index;
}

template<PredicateCategory P>
Predicate<P> ConceptPredicateGoalImpl<P>::get_predicate() const
{
    return m_predicate;
}

template class ConceptPredicateGoalImpl<Static>;
template class ConceptPredicateGoalImpl<Fluent>;
template class ConceptPredicateGoalImpl<Derived>;

/**
 * ConceptAnd
 */
ConceptAndImpl::ConceptAndImpl(Index index, Choice<Concept> concept_left, Choice<Concept> concept_right) :
    m_index(index),
    m_concept_left(concept_left),
    m_concept_right(concept_right)
{
}

bool ConceptAndImpl::test_match(dl::Constructor<Concept> constructor) const { return constructor->accept(ConceptAndVisitor(this)); }

Index ConceptAndImpl::get_index() const { return m_index; }

Choice<Concept> ConceptAndImpl::get_concept_left() const { return m_concept_left; }

Choice<Concept> ConceptAndImpl::get_concept_right() const { return m_concept_right; }

/**
 * RolePredicateState
 */

template<PredicateCategory P>
RolePredicateStateImpl<P>::RolePredicateStateImpl(Index index, Predicate<P> predicate) : m_index(index), m_predicate(predicate)
{
}

template<PredicateCategory P>
bool RolePredicateStateImpl<P>::test_match(dl::Constructor<Role> constructor) const
{
    return constructor->accept(RolePredicateStateVisitor<P>(this));
}

template<PredicateCategory P>
Index RolePredicateStateImpl<P>::get_index() const
{
    return m_index;
}

template<PredicateCategory P>
Predicate<P> RolePredicateStateImpl<P>::get_predicate() const
{
    return m_predicate;
}

template class RolePredicateStateImpl<Static>;
template class RolePredicateStateImpl<Fluent>;
template class RolePredicateStateImpl<Derived>;

/**
 * RolePredicateGoal
 */

template<PredicateCategory P>
RolePredicateGoalImpl<P>::RolePredicateGoalImpl(Index index, Predicate<P> predicate) : m_index(index), m_predicate(predicate)
{
}

template<PredicateCategory P>
bool RolePredicateGoalImpl<P>::test_match(dl::Constructor<Role> constructor) const
{
    return constructor->accept(RolePredicateGoalVisitor<P>(this));
}

template<PredicateCategory P>
Index RolePredicateGoalImpl<P>::get_index() const
{
    return m_index;
}

template<PredicateCategory P>
Predicate<P> RolePredicateGoalImpl<P>::get_predicate() const
{
    return m_predicate;
}

template class RolePredicateGoalImpl<Static>;
template class RolePredicateGoalImpl<Fluent>;
template class RolePredicateGoalImpl<Derived>;

/**
 * RoleAnd
 */
RoleAndImpl::RoleAndImpl(Index index, Choice<Role> role_left, Choice<Role> role_right) : m_index(index), m_role_left(role_left), m_role_right(role_right) {}

bool RoleAndImpl::test_match(dl::Constructor<Role> constructor) const { return constructor->accept(RoleAndVisitor(this)); }

Index RoleAndImpl::get_index() const { return m_index; }

Choice<Role> RoleAndImpl::get_role_left() const { return m_role_left; }

Choice<Role> RoleAndImpl::get_role_right() const { return m_role_right; }

}
