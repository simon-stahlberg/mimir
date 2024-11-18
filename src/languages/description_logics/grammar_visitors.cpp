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
 * ConceptBotVisitor
 */

ConceptBotVisitor::ConceptBotVisitor(ConceptBot grammar_constructor) : m_grammar_constructor(grammar_constructor) {}

bool ConceptBotVisitor::visit(dl::ConceptBot constructor) const { return true; }

/**
 * ConceptTopVisitor
 */

ConceptTopVisitor::ConceptTopVisitor(ConceptTop grammar_constructor) : m_grammar_constructor(grammar_constructor) {}

bool ConceptTopVisitor::visit(dl::ConceptTop constructor) const { return true; }

/**
 * ConceptAtomicStateVisitor
 */

template<PredicateTag P>
ConceptAtomicStateVisitor<P>::ConceptAtomicStateVisitor(ConceptAtomicState<P> grammar_constructor) : m_grammar_constructor(grammar_constructor)
{
}

template<PredicateTag P>
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

template<PredicateTag P>
ConceptAtomicGoalVisitor<P>::ConceptAtomicGoalVisitor(ConceptAtomicGoal<P> grammar_constructor) : m_grammar_constructor(grammar_constructor)
{
}

template<PredicateTag P>
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
 * ConceptNegationVisitor
 */

ConceptNegationVisitor::ConceptNegationVisitor(ConceptNegation grammar_constructor) : m_grammar_constructor(grammar_constructor) {}

bool ConceptNegationVisitor::visit(dl::ConceptNegation constructor) const
{
    return m_grammar_constructor->get_concept()->test_match(constructor->get_concept());
}

/**
 * ConceptValueRestrictionVisitor
 */

ConceptValueRestrictionVisitor::ConceptValueRestrictionVisitor(ConceptValueRestriction grammar_constructor) : m_grammar_constructor(grammar_constructor) {}

bool ConceptValueRestrictionVisitor::visit(dl::ConceptValueRestriction constructor) const
{
    return m_grammar_constructor->get_role()->test_match(constructor->get_role())  //
           && m_grammar_constructor->get_concept()->test_match(constructor->get_concept());
}

/**
 * ConceptExistentialQuantificationVisitor
 */

ConceptExistentialQuantificationVisitor::ConceptExistentialQuantificationVisitor(ConceptExistentialQuantification grammar_constructor) :
    m_grammar_constructor(grammar_constructor)
{
}

bool ConceptExistentialQuantificationVisitor::visit(dl::ConceptExistentialQuantification constructor) const
{
    return m_grammar_constructor->get_role()->test_match(constructor->get_role())  //
           && m_grammar_constructor->get_concept()->test_match(constructor->get_concept());
}

/**
 * ConceptRoleValueMapContainmentVisitor
 */

ConceptRoleValueMapContainmentVisitor::ConceptRoleValueMapContainmentVisitor(ConceptRoleValueMapContainment grammar_constructor) :
    m_grammar_constructor(grammar_constructor)
{
}

bool ConceptRoleValueMapContainmentVisitor::visit(dl::ConceptRoleValueMapContainment constructor) const
{
    return m_grammar_constructor->get_role_left()->test_match(constructor->get_role_left())  //
           && m_grammar_constructor->get_role_right()->test_match(constructor->get_role_right());
}

/**
 * ConceptRoleValueMapEqualityVisitor
 */

ConceptRoleValueMapEqualityVisitor::ConceptRoleValueMapEqualityVisitor(ConceptRoleValueMapEquality grammar_constructor) :
    m_grammar_constructor(grammar_constructor)
{
}

bool ConceptRoleValueMapEqualityVisitor::visit(dl::ConceptRoleValueMapEquality constructor) const
{
    return m_grammar_constructor->get_role_left()->test_match(constructor->get_role_left())  //
           && m_grammar_constructor->get_role_right()->test_match(constructor->get_role_right());
}

/**
 * ConceptNominalVisitor
 */

ConceptNominalVisitor::ConceptNominalVisitor(ConceptNominal grammar_constructor) : m_grammar_constructor(grammar_constructor) {}

bool ConceptNominalVisitor::visit(dl::ConceptNominal constructor) const { return m_grammar_constructor->get_object() == constructor->get_object(); }

/**
 * RoleUniversalVisitor
 */

RoleUniversalVisitor::RoleUniversalVisitor(RoleUniversal grammar_constructor) : m_grammar_constructor(grammar_constructor) {}

bool RoleUniversalVisitor::visit(dl::RoleUniversal constructor) const { return true; }

/**
 * RoleAtomicStateVisitor
 */

template<PredicateTag P>
RoleAtomicStateVisitor<P>::RoleAtomicStateVisitor(RoleAtomicState<P> grammar_constructor) : m_grammar_constructor(grammar_constructor)
{
}

template<PredicateTag P>
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

template<PredicateTag P>
RoleAtomicGoalVisitor<P>::RoleAtomicGoalVisitor(RoleAtomicGoal<P> grammar_constructor) : m_grammar_constructor(grammar_constructor)
{
}

template<PredicateTag P>
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

/**
 * RoleUnionVisitor
 */

RoleUnionVisitor::RoleUnionVisitor(RoleUnion grammar_constructor) : m_grammar_constructor(grammar_constructor) {}

bool RoleUnionVisitor::visit(dl::RoleUnion constructor) const
{
    return m_grammar_constructor->get_role_left()->test_match(constructor->get_role_left())
           && m_grammar_constructor->get_role_right()->test_match(constructor->get_role_right());
}

/**
 * RoleComplementVisitor
 */

RoleComplementVisitor::RoleComplementVisitor(RoleComplement grammar_constructor) : m_grammar_constructor(grammar_constructor) {}

bool RoleComplementVisitor::visit(dl::RoleComplement constructor) const { return m_grammar_constructor->get_role()->test_match(constructor->get_role()); }

/**
 * RoleInverseVisitor
 */

RoleInverseVisitor::RoleInverseVisitor(RoleInverse grammar_constructor) : m_grammar_constructor(grammar_constructor) {}

bool RoleInverseVisitor::visit(dl::RoleInverse constructor) const { return m_grammar_constructor->get_role()->test_match(constructor->get_role()); }

/**
 * RoleCompositionVisitor
 */

RoleCompositionVisitor::RoleCompositionVisitor(RoleComposition grammar_constructor) : m_grammar_constructor(grammar_constructor) {}

bool RoleCompositionVisitor::visit(dl::RoleComposition constructor) const
{
    return m_grammar_constructor->get_role_left()->test_match(constructor->get_role_left())
           && m_grammar_constructor->get_role_right()->test_match(constructor->get_role_right());
}

/**
 * RoleTransitiveClosureVisitor
 */

RoleTransitiveClosureVisitor::RoleTransitiveClosureVisitor(RoleTransitiveClosure grammar_constructor) : m_grammar_constructor(grammar_constructor) {}

bool RoleTransitiveClosureVisitor::visit(dl::RoleTransitiveClosure constructor) const
{
    return m_grammar_constructor->get_role()->test_match(constructor->get_role());
}

/**
 * RoleReflexiveTransitiveClosureVisitor
 */

RoleReflexiveTransitiveClosureVisitor::RoleReflexiveTransitiveClosureVisitor(RoleReflexiveTransitiveClosure grammar_constructor) :
    m_grammar_constructor(grammar_constructor)
{
}

bool RoleReflexiveTransitiveClosureVisitor::visit(dl::RoleReflexiveTransitiveClosure constructor) const
{
    return m_grammar_constructor->get_role()->test_match(constructor->get_role());
}

/**
 * RoleRestrictionVisitor
 */

RoleRestrictionVisitor::RoleRestrictionVisitor(RoleRestriction grammar_constructor) : m_grammar_constructor(grammar_constructor) {}

bool RoleRestrictionVisitor::visit(dl::RoleRestriction constructor) const
{
    return m_grammar_constructor->get_role()->test_match(constructor->get_role())
           && m_grammar_constructor->get_concept()->test_match(constructor->get_concept());
}

/**
 * RoleIdentityVisitor
 */

RoleIdentityVisitor::RoleIdentityVisitor(RoleIdentity grammar_constructor) : m_grammar_constructor(grammar_constructor) {}

bool RoleIdentityVisitor::visit(dl::RoleIdentity constructor) const { return m_grammar_constructor->get_concept()->test_match(constructor->get_concept()); }

}