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

#include <mimir/languages/description_logics/grammar_visitors.hpp>

namespace mimir::dl::grammar
{

/**
 * ConceptPredicateStateVisitor
 */

template<PredicateCategory P>
ConceptPredicateStateVisitor<P>::ConceptPredicateStateVisitor(const ConceptPredicateState<P>& grammar_constructor) : m_grammar_constructor(grammar_constructor)
{
}

template<PredicateCategory P>
bool ConceptPredicateStateVisitor<P>::visit(const dl::ConceptPredicateState<P>& constructor) const
{
    return constructor.get_predicate() == m_grammar_constructor.get_predicate();
}

template class ConceptPredicateStateVisitor<Static>;
template class ConceptPredicateStateVisitor<Fluent>;
template class ConceptPredicateStateVisitor<Derived>;

/**
 * ConceptPredicateGoalVisitor
 */

template<PredicateCategory P>
ConceptPredicateGoalVisitor<P>::ConceptPredicateGoalVisitor(const ConceptPredicateGoal<P>& grammar_constructor) : m_grammar_constructor(grammar_constructor)
{
}

template<PredicateCategory P>
bool ConceptPredicateGoalVisitor<P>::visit(const dl::ConceptPredicateGoal<P>& constructor) const
{
    return constructor.get_predicate() == m_grammar_constructor.get_predicate();
}

template class ConceptPredicateGoalVisitor<Static>;
template class ConceptPredicateGoalVisitor<Fluent>;
template class ConceptPredicateGoalVisitor<Derived>;

/**
 * ConceptAndVisitor
 */

ConceptAndVisitor::ConceptAndVisitor(const ConceptAnd& grammar_constructor) : m_grammar_constructor(grammar_constructor) {}

bool ConceptAndVisitor::visit(const dl::ConceptAnd& constructor) const
{
    return m_grammar_constructor.get_concept_left().test_match(constructor.get_concept_left())  //
           && m_grammar_constructor.get_concept_right().test_match(constructor.get_concept_right());
}

/**
 * RolePredicateStateVisitor
 */

template<PredicateCategory P>
RolePredicateStateVisitor<P>::RolePredicateStateVisitor(const RolePredicateState<P>& grammar_constructor) : m_grammar_constructor(grammar_constructor)
{
}

template<PredicateCategory P>
bool RolePredicateStateVisitor<P>::visit(const dl::RolePredicateState<P>& constructor) const
{
    return constructor.get_predicate() == m_grammar_constructor.get_predicate();
}

template class RolePredicateStateVisitor<Static>;
template class RolePredicateStateVisitor<Fluent>;
template class RolePredicateStateVisitor<Derived>;

/**
 * RolePredicateGoalVisitor
 */

template<PredicateCategory P>
RolePredicateGoalVisitor<P>::RolePredicateGoalVisitor(const RolePredicateGoal<P>& grammar_constructor) : m_grammar_constructor(grammar_constructor)
{
}

template<PredicateCategory P>
bool RolePredicateGoalVisitor<P>::visit(const dl::RolePredicateGoal<P>& constructor) const
{
    return constructor.get_predicate() == m_grammar_constructor.get_predicate();
}

template class RolePredicateGoalVisitor<Static>;
template class RolePredicateGoalVisitor<Fluent>;
template class RolePredicateGoalVisitor<Derived>;

/**
 * RoleAndVisitor
 */

RoleAndVisitor::RoleAndVisitor(const RoleAnd& grammar_constructor) : m_grammar_constructor(grammar_constructor) {}

bool RoleAndVisitor::visit(const dl::RoleAnd& constructor) const
{
    return m_grammar_constructor.get_role_left().test_match(constructor.get_role_left())
           && m_grammar_constructor.get_role_right().test_match(constructor.get_role_right());
}
}