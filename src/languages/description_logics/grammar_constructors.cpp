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

#include "mimir/languages/description_logics/visitors.hpp"

#include <mimir/languages/description_logics/grammar_constructors.hpp>

namespace mimir::dl::grammar
{

/**
 * ConceptPredicateState
 */

template<PredicateCategory P>
ConceptPredicateState<P>::ConceptPredicateState(Predicate<P> predicate) : m_predicate(predicate)
{
}

template<PredicateCategory P>
bool ConceptPredicateState<P>::test_match(const dl::Concept& constructor) const
{
    return constructor.accept(ConceptPredicateStateVisitor(this));
}

template<PredicateCategory P>
Predicate<P> ConceptPredicateState<P>::get_predicate() const
{
}

/**
 * ConceptPredicateGoal
 */

template<PredicateCategory P>
ConceptPredicateGoal<P>::ConceptPredicateGoal(Predicate<P> predicate) : m_predicate(predicate)
{
}

template<PredicateCategory P>
bool ConceptPredicateGoal<P>::test_match(const dl::Concept& constructor) const
{
    return constructor.accept(ConceptPredicateGoalVisitor(this));
}

template<PredicateCategory P>
Predicate<P> ConceptPredicateGoal<P>::get_predicate() const
{
}

/**
 * ConceptAnd
 */
ConceptAnd::ConceptAnd(const Constructor<Concept>& concept_left, const Constructor<Concept>& concept_right) :
    m_concept_left(concept_left),
    m_concept_right(concept_right)
{
}

bool ConceptAnd::test_match(const dl::Concept& constructor) const { return constructor.accept(ConceptAndVisitor(*this)); }

const Constructor<Concept>& ConceptAnd::get_concept_left() const { return m_concept_left; }

const Constructor<Concept>& ConceptAnd::get_concept_right() const { return m_concept_right; }

/**
 * RolePredicateState
 */

template<PredicateCategory P>
RolePredicateState<P>::RolePredicateState(Predicate<P> predicate) : m_predicate(predicate)
{
}

template<PredicateCategory P>
bool RolePredicateState<P>::test_match(const dl::Role& constructor) const
{
    return constructor.accept(RolePredicateStateVisitor(this));
}

template<PredicateCategory P>
Predicate<P> RolePredicateState<P>::get_predicate() const
{
}

/**
 * RolePredicateGoal
 */

template<PredicateCategory P>
RolePredicateGoal<P>::RolePredicateGoal(Predicate<P> predicate) : m_predicate(predicate)
{
}

template<PredicateCategory P>
bool RolePredicateGoal<P>::test_match(const dl::Role& constructor) const
{
    return constructor.accept(RolePredicateGoalVisitor(this));
}

template<PredicateCategory P>
Predicate<P> RolePredicateGoal<P>::get_predicate() const
{
}

/**
 * RoleAnd
 */
RoleAnd::RoleAnd(const Constructor<Role>& role_left, const Constructor<Role>& role_right) : m_role_left(role_left), m_role_right(role_right) {}

bool RoleAnd::test_match(const dl::Role& constructor) const { return constructor.accept(RoleAndVisitor(*this)); }

const Constructor<Role>& RoleAnd::get_role_left() const { return m_role_left; }

const Constructor<Role>& RoleAnd::get_role_right() const { return m_role_right; }

}
