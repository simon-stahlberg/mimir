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

#include <mimir/languages/description_logics/visitors.hpp>

namespace mimir::dl
{

/**
 * ConceptPredicateStateVisitor
 */

template<PredicateCategory P>
ConceptPredicateStateVisitor<P>::ConceptPredicateStateVisitor(const grammar::ConceptPredicateState<P>& grammar_constructor) :
    m_grammar_constructor(grammar_constructor)
{
}

template<PredicateCategory P>
bool ConceptPredicateStateVisitor<P>::visit(const ConceptPredicateState<P>& constructor) const
{
    return constructor.get_predicate() == m_grammar_constructor->get_predicate();
}

/**
 * ConceptPredicateGoalVisitor
 */

template<PredicateCategory P>
ConceptPredicateGoalVisitor<P>::ConceptPredicateGoalVisitor(const grammar::ConceptPredicateGoal<P>& grammar_constructor) :
    m_grammar_constructor(grammar_constructor)
{
}

template<PredicateCategory P>
bool ConceptPredicateGoalVisitor<P>::visit(const ConceptPredicateGoal<P>& constructor) const
{
    return constructor.get_predicate() == m_grammar_constructor->get_predicate();
}

/**
 * ConceptAndVisitor
 */

ConceptAndVisitor::ConceptAndVisitor(const grammar::ConceptAnd& grammar_constructor) : m_grammar_constructor(grammar_constructor) {}

bool ConceptAndVisitor::visit(const ConceptAnd& constructor) const
{
    return m_grammar_constructor.get_concept_left().test_match(constructor.get_concept_left())
           && m_grammar_constructor.get_concept_right().test_match(constructor.get_concept_right());
}

/**
 * RolePredicateStateVisitor
 */

template<PredicateCategory P>
RolePredicateStateVisitor<P>::RolePredicateStateVisitor(const grammar::RolePredicateState<P>& grammar_constructor) : m_grammar_constructor(grammar_constructor)
{
}

template<PredicateCategory P>
bool RolePredicateStateVisitor<P>::visit(const RolePredicateState<P>& constructor) const
{
    return constructor.get_predicate() == m_grammar_constructor->get_predicate();
}

/**
 * RolePredicateGoalVisitor
 */

template<PredicateCategory P>
RolePredicateGoalVisitor<P>::RolePredicateGoalVisitor(const grammar::RolePredicateGoal<P>& grammar_constructor) : m_grammar_constructor(grammar_constructor)
{
}

template<PredicateCategory P>
bool RolePredicateGoalVisitor<P>::visit(const RolePredicateGoal<P>& constructor) const
{
    return constructor.get_predicate() == m_grammar_constructor->get_predicate();
}

/**
 * RoleAndVisitor
 */

RoleAndVisitor::RoleAndVisitor(const grammar::RoleAnd& grammar_constructor) : m_grammar_constructor(grammar_constructor) {}

bool RoleAndVisitor::visit(const RoleAnd& constructor) const
{
    return m_grammar_constructor.get_role_left().test_match(constructor.get_role_left())
           && m_grammar_constructor.get_role_right().test_match(constructor.get_role_right());
}
}
