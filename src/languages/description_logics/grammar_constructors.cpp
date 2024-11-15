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
 * ConceptAtomicStateImpl
 */

template<PredicateCategory P>
ConceptAtomicStateImpl<P>::ConceptAtomicStateImpl(Index index, Predicate<P> predicate) : m_index(index), m_predicate(predicate)
{
}

template<PredicateCategory P>
bool ConceptAtomicStateImpl<P>::test_match(dl::Constructor<Concept> constructor) const
{
    return constructor->accept(grammar::ConceptAtomicStateVisitor<P>(this));
}

template<PredicateCategory P>
Index ConceptAtomicStateImpl<P>::get_index() const
{
    return m_index;
}

template<PredicateCategory P>
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

template<PredicateCategory P>
ConceptAtomicGoalImpl<P>::ConceptAtomicGoalImpl(Index index, Predicate<P> predicate) : m_index(index), m_predicate(predicate)
{
}

template<PredicateCategory P>
bool ConceptAtomicGoalImpl<P>::test_match(dl::Constructor<Concept> constructor) const
{
    return constructor->accept(ConceptAtomicGoalVisitor<P>(this));
}

template<PredicateCategory P>
Index ConceptAtomicGoalImpl<P>::get_index() const
{
    return m_index;
}

template<PredicateCategory P>
Predicate<P> ConceptAtomicGoalImpl<P>::get_predicate() const
{
    return m_predicate;
}

template class ConceptAtomicGoalImpl<Static>;
template class ConceptAtomicGoalImpl<Fluent>;
template class ConceptAtomicGoalImpl<Derived>;

/**
 * ConceptIntersection
 */
ConceptIntersectionImpl::ConceptIntersectionImpl(Index index, Choice<Concept> concept_left, Choice<Concept> concept_right) :
    m_index(index),
    m_concept_left(concept_left),
    m_concept_right(concept_right)
{
}

bool ConceptIntersectionImpl::test_match(dl::Constructor<Concept> constructor) const { return constructor->accept(ConceptIntersectionVisitor(this)); }

Index ConceptIntersectionImpl::get_index() const { return m_index; }

Choice<Concept> ConceptIntersectionImpl::get_concept_left() const { return m_concept_left; }

Choice<Concept> ConceptIntersectionImpl::get_concept_right() const { return m_concept_right; }

/**
 * ConceptUnion
 */
ConceptUnionImpl::ConceptUnionImpl(Index index, Choice<Concept> concept_left, Choice<Concept> concept_right) :
    m_index(index),
    m_concept_left(concept_left),
    m_concept_right(concept_right)
{
}

bool ConceptUnionImpl::test_match(dl::Constructor<Concept> constructor) const { return constructor->accept(ConceptUnionVisitor(this)); }

Index ConceptUnionImpl::get_index() const { return m_index; }

Choice<Concept> ConceptUnionImpl::get_concept_left() const { return m_concept_left; }

Choice<Concept> ConceptUnionImpl::get_concept_right() const { return m_concept_right; }

/**
 * RoleAtomicState
 */

template<PredicateCategory P>
RoleAtomicStateImpl<P>::RoleAtomicStateImpl(Index index, Predicate<P> predicate) : m_index(index), m_predicate(predicate)
{
}

template<PredicateCategory P>
bool RoleAtomicStateImpl<P>::test_match(dl::Constructor<Role> constructor) const
{
    return constructor->accept(RoleAtomicStateVisitor<P>(this));
}

template<PredicateCategory P>
Index RoleAtomicStateImpl<P>::get_index() const
{
    return m_index;
}

template<PredicateCategory P>
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

template<PredicateCategory P>
RoleAtomicGoalImpl<P>::RoleAtomicGoalImpl(Index index, Predicate<P> predicate) : m_index(index), m_predicate(predicate)
{
}

template<PredicateCategory P>
bool RoleAtomicGoalImpl<P>::test_match(dl::Constructor<Role> constructor) const
{
    return constructor->accept(RoleAtomicGoalVisitor<P>(this));
}

template<PredicateCategory P>
Index RoleAtomicGoalImpl<P>::get_index() const
{
    return m_index;
}

template<PredicateCategory P>
Predicate<P> RoleAtomicGoalImpl<P>::get_predicate() const
{
    return m_predicate;
}

template class RoleAtomicGoalImpl<Static>;
template class RoleAtomicGoalImpl<Fluent>;
template class RoleAtomicGoalImpl<Derived>;

/**
 * RoleIntersection
 */
RoleIntersectionImpl::RoleIntersectionImpl(Index index, Choice<Role> role_left, Choice<Role> role_right) :
    m_index(index),
    m_role_left(role_left),
    m_role_right(role_right)
{
}

bool RoleIntersectionImpl::test_match(dl::Constructor<Role> constructor) const { return constructor->accept(RoleIntersectionVisitor(this)); }

Index RoleIntersectionImpl::get_index() const { return m_index; }

Choice<Role> RoleIntersectionImpl::get_role_left() const { return m_role_left; }

Choice<Role> RoleIntersectionImpl::get_role_right() const { return m_role_right; }

}
