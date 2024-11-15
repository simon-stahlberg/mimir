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

#include "mimir/languages/description_logics/grammar_visitors.hpp"

#include "mimir/languages/description_logics/constructors.hpp"
#include "mimir/languages/description_logics/grammar_constructors.hpp"

namespace mimir::dl::grammar
{

/**
 * ConceptAtomicStateVisitor
 */

template<PredicateCategory P>
ConceptAtomicStateVisitor<P>::ConceptAtomicStateVisitor(ConceptAtomicState<P> grammar_constructor) : m_grammar_constructor(grammar_constructor)
{
}

template<PredicateCategory P>
bool ConceptAtomicStateVisitor<P>::visit(dl::ConceptAtomicState<P> constructor) const
{
    return constructor->get_predicate() == m_grammar_constructor->get_predicate();
}

template class ConceptAtomicStateVisitor<Static>;
template class ConceptAtomicStateVisitor<Fluent>;
template class ConceptAtomicStateVisitor<Derived>;

/**
 * ConceptAtomicGoalVisitor
 */

template<PredicateCategory P>
ConceptAtomicGoalVisitor<P>::ConceptAtomicGoalVisitor(ConceptAtomicGoal<P> grammar_constructor) : m_grammar_constructor(grammar_constructor)
{
}

template<PredicateCategory P>
bool ConceptAtomicGoalVisitor<P>::visit(dl::ConceptAtomicGoal<P> constructor) const
{
    return constructor->get_predicate() == m_grammar_constructor->get_predicate();
}

template class ConceptAtomicGoalVisitor<Static>;
template class ConceptAtomicGoalVisitor<Fluent>;
template class ConceptAtomicGoalVisitor<Derived>;

/**
 * ConceptIntersectionVisitor
 */

ConceptIntersectionVisitor::ConceptIntersectionVisitor(ConceptIntersection grammar_constructor) : m_grammar_constructor(grammar_constructor) {}

bool ConceptIntersectionVisitor::visit(dl::ConceptIntersection constructor) const
{
    return m_grammar_constructor->get_concept_left()->test_match(constructor->get_concept_left())  //
           && m_grammar_constructor->get_concept_right()->test_match(constructor->get_concept_right());
}

/**
 * ConceptUnionVisitor
 */

ConceptUnionVisitor::ConceptUnionVisitor(ConceptUnion grammar_constructor) : m_grammar_constructor(grammar_constructor) {}

bool ConceptUnionVisitor::visit(dl::ConceptUnion constructor) const
{
    return m_grammar_constructor->get_concept_left()->test_match(constructor->get_concept_left())  //
           && m_grammar_constructor->get_concept_right()->test_match(constructor->get_concept_right());
}

/**
 * RoleAtomicStateVisitor
 */

template<PredicateCategory P>
RoleAtomicStateVisitor<P>::RoleAtomicStateVisitor(RoleAtomicState<P> grammar_constructor) : m_grammar_constructor(grammar_constructor)
{
}

template<PredicateCategory P>
bool RoleAtomicStateVisitor<P>::visit(dl::RoleAtomicState<P> constructor) const
{
    return constructor->get_predicate() == m_grammar_constructor->get_predicate();
}

template class RoleAtomicStateVisitor<Static>;
template class RoleAtomicStateVisitor<Fluent>;
template class RoleAtomicStateVisitor<Derived>;

/**
 * RoleAtomicGoalVisitor
 */

template<PredicateCategory P>
RoleAtomicGoalVisitor<P>::RoleAtomicGoalVisitor(RoleAtomicGoal<P> grammar_constructor) : m_grammar_constructor(grammar_constructor)
{
}

template<PredicateCategory P>
bool RoleAtomicGoalVisitor<P>::visit(dl::RoleAtomicGoal<P> constructor) const
{
    return constructor->get_predicate() == m_grammar_constructor->get_predicate();
}

template class RoleAtomicGoalVisitor<Static>;
template class RoleAtomicGoalVisitor<Fluent>;
template class RoleAtomicGoalVisitor<Derived>;

/**
 * RoleIntersectionVisitor
 */

RoleIntersectionVisitor::RoleIntersectionVisitor(RoleIntersection grammar_constructor) : m_grammar_constructor(grammar_constructor) {}

bool RoleIntersectionVisitor::visit(dl::RoleIntersection constructor) const
{
    return m_grammar_constructor->get_role_left()->test_match(constructor->get_role_left())
           && m_grammar_constructor->get_role_right()->test_match(constructor->get_role_right());
}
}