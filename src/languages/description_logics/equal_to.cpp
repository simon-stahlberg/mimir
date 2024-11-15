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
 *<
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "mimir/languages/description_logics/equal_to.hpp"

#include "mimir/languages/description_logics/constructors.hpp"
#include "mimir/languages/description_logics/grammar_constructors.hpp"

namespace mimir::dl
{

/**
 * DL constructors
 */

/* Concepts */

template<PredicateCategory P>
bool UniqueDLEqualTo<const ConceptAtomicStateImpl<P>*>::operator()(const ConceptAtomicStateImpl<P>* l, const ConceptAtomicStateImpl<P>* r) const
{
    if (&l != &r)
    {
        return (l->get_predicate() == r->get_predicate());
    }
    return true;
}

template bool UniqueDLEqualTo<const ConceptAtomicStateImpl<Static>*>::operator()(const ConceptAtomicStateImpl<Static>* l,
                                                                                 const ConceptAtomicStateImpl<Static>* r) const;
template bool UniqueDLEqualTo<const ConceptAtomicStateImpl<Fluent>*>::operator()(const ConceptAtomicStateImpl<Fluent>* l,
                                                                                 const ConceptAtomicStateImpl<Fluent>* r) const;
template bool UniqueDLEqualTo<const ConceptAtomicStateImpl<Derived>*>::operator()(const ConceptAtomicStateImpl<Derived>* l,
                                                                                  const ConceptAtomicStateImpl<Derived>* r) const;

template<PredicateCategory P>
bool UniqueDLEqualTo<const ConceptAtomicGoalImpl<P>*>::operator()(const ConceptAtomicGoalImpl<P>* l, const ConceptAtomicGoalImpl<P>* r) const
{
    if (&l != &r)
    {
        return (l->get_predicate() == r->get_predicate());
    }
    return true;
}

template bool UniqueDLEqualTo<const ConceptAtomicGoalImpl<Static>*>::operator()(const ConceptAtomicGoalImpl<Static>* l,
                                                                                const ConceptAtomicGoalImpl<Static>* r) const;
template bool UniqueDLEqualTo<const ConceptAtomicGoalImpl<Fluent>*>::operator()(const ConceptAtomicGoalImpl<Fluent>* l,
                                                                                const ConceptAtomicGoalImpl<Fluent>* r) const;
template bool UniqueDLEqualTo<const ConceptAtomicGoalImpl<Derived>*>::operator()(const ConceptAtomicGoalImpl<Derived>* l,
                                                                                 const ConceptAtomicGoalImpl<Derived>* r) const;

bool UniqueDLEqualTo<const ConceptIntersectionImpl*>::operator()(const ConceptIntersectionImpl* l, const ConceptIntersectionImpl* r) const
{
    if (&l != &r)
    {
        return (l->get_concept_left() == r->get_concept_left()) && (l->get_concept_right() == r->get_concept_right());
    }
    return true;
}

/* Roles */

template<PredicateCategory P>
bool UniqueDLEqualTo<const RoleAtomicStateImpl<P>*>::operator()(const RoleAtomicStateImpl<P>* l, const RoleAtomicStateImpl<P>* r) const
{
    if (&l != &r)
    {
        return (l->get_predicate() == r->get_predicate());
    }
    return true;
}

template bool UniqueDLEqualTo<const RoleAtomicStateImpl<Static>*>::operator()(const RoleAtomicStateImpl<Static>* l, const RoleAtomicStateImpl<Static>* r) const;
template bool UniqueDLEqualTo<const RoleAtomicStateImpl<Fluent>*>::operator()(const RoleAtomicStateImpl<Fluent>* l, const RoleAtomicStateImpl<Fluent>* r) const;
template bool UniqueDLEqualTo<const RoleAtomicStateImpl<Derived>*>::operator()(const RoleAtomicStateImpl<Derived>* l,
                                                                               const RoleAtomicStateImpl<Derived>* r) const;

template<PredicateCategory P>
bool UniqueDLEqualTo<const RoleAtomicGoalImpl<P>*>::operator()(const RoleAtomicGoalImpl<P>* l, const RoleAtomicGoalImpl<P>* r) const
{
    if (&l != &r)
    {
        return (l->get_predicate() == r->get_predicate());
    }
    return true;
}

template bool UniqueDLEqualTo<const RoleAtomicGoalImpl<Static>*>::operator()(const RoleAtomicGoalImpl<Static>* l, const RoleAtomicGoalImpl<Static>* r) const;
template bool UniqueDLEqualTo<const RoleAtomicGoalImpl<Fluent>*>::operator()(const RoleAtomicGoalImpl<Fluent>* l, const RoleAtomicGoalImpl<Fluent>* r) const;
template bool UniqueDLEqualTo<const RoleAtomicGoalImpl<Derived>*>::operator()(const RoleAtomicGoalImpl<Derived>* l, const RoleAtomicGoalImpl<Derived>* r) const;

bool UniqueDLEqualTo<const RoleIntersectionImpl*>::operator()(const RoleIntersectionImpl* l, const RoleIntersectionImpl* r) const
{
    if (&l != &r)
    {
        return (l->get_role_left() == r->get_role_left()) && (l->get_role_right() == r->get_role_right());
    }
    return true;
}

/**
 * DL grammar constructors
 */

template<IsConceptOrRole D>
bool UniqueDLEqualTo<const grammar::DerivationRuleImpl<D>*>::operator()(const grammar::DerivationRuleImpl<D>* l, const grammar::DerivationRuleImpl<D>* r) const
{
    if (&l != &r)
    {
        return (l->get_choices() == r->get_choices());
    }
    return true;
}

template bool UniqueDLEqualTo<const grammar::DerivationRuleImpl<Concept>*>::operator()(const grammar::DerivationRuleImpl<Concept>* l,
                                                                                       const grammar::DerivationRuleImpl<Concept>* r) const;
template bool UniqueDLEqualTo<const grammar::DerivationRuleImpl<Role>*>::operator()(const grammar::DerivationRuleImpl<Role>* l,
                                                                                    const grammar::DerivationRuleImpl<Role>* r) const;

template<IsConceptOrRole D>
bool UniqueDLEqualTo<const grammar::NonTerminalImpl<D>*>::operator()(const grammar::NonTerminalImpl<D>* l, const grammar::NonTerminalImpl<D>* r) const
{
    if (&l != &r)
    {
        return (l->get_name() == r->get_name());
    }
    return true;
}

template bool UniqueDLEqualTo<const grammar::NonTerminalImpl<Concept>*>::operator()(const grammar::NonTerminalImpl<Concept>* l,
                                                                                    const grammar::NonTerminalImpl<Concept>* r) const;
template bool UniqueDLEqualTo<const grammar::NonTerminalImpl<Role>*>::operator()(const grammar::NonTerminalImpl<Role>* l,
                                                                                 const grammar::NonTerminalImpl<Role>* r) const;

template<IsConceptOrRole D>
bool UniqueDLEqualTo<const grammar::ConstructorOrNonTerminalChoice<D>*>::operator()(const grammar::ConstructorOrNonTerminalChoice<D>* l,
                                                                                    const grammar::ConstructorOrNonTerminalChoice<D>* r) const
{
    if (&l != &r)
    {
        return (*l == *r);
    }
    return true;
}

template bool
UniqueDLEqualTo<const grammar::ConstructorOrNonTerminalChoice<Concept>*>::operator()(const grammar::ConstructorOrNonTerminalChoice<Concept>* l,
                                                                                     const grammar::ConstructorOrNonTerminalChoice<Concept>* r) const;
template bool UniqueDLEqualTo<const grammar::ConstructorOrNonTerminalChoice<Role>*>::operator()(const grammar::ConstructorOrNonTerminalChoice<Role>* l,
                                                                                                const grammar::ConstructorOrNonTerminalChoice<Role>* r) const;

template<IsConceptOrRole D>
bool UniqueDLEqualTo<const grammar::ChoiceImpl<D>*>::operator()(const grammar::ChoiceImpl<D>* l, const grammar::ChoiceImpl<D>* r) const
{
    if (&l != &r)
    {
        return (l->get_choice() == r->get_choice());
    }
    return true;
}

template bool UniqueDLEqualTo<const grammar::ChoiceImpl<Concept>*>::operator()(const grammar::ChoiceImpl<Concept>* l,
                                                                               const grammar::ChoiceImpl<Concept>* r) const;
template bool UniqueDLEqualTo<const grammar::ChoiceImpl<Role>*>::operator()(const grammar::ChoiceImpl<Role>* l, const grammar::ChoiceImpl<Role>* r) const;

/* Concepts */

template<PredicateCategory P>
bool UniqueDLEqualTo<const grammar::ConceptAtomicStateImpl<P>*>::operator()(const grammar::ConceptAtomicStateImpl<P>* l,
                                                                            const grammar::ConceptAtomicStateImpl<P>* r) const
{
    if (&l != &r)
    {
        return (l->get_predicate() == r->get_predicate());
    }
    return true;
}

template bool UniqueDLEqualTo<const grammar::ConceptAtomicStateImpl<Static>*>::operator()(const grammar::ConceptAtomicStateImpl<Static>* l,
                                                                                          const grammar::ConceptAtomicStateImpl<Static>* r) const;
template bool UniqueDLEqualTo<const grammar::ConceptAtomicStateImpl<Fluent>*>::operator()(const grammar::ConceptAtomicStateImpl<Fluent>* l,
                                                                                          const grammar::ConceptAtomicStateImpl<Fluent>* r) const;
template bool UniqueDLEqualTo<const grammar::ConceptAtomicStateImpl<Derived>*>::operator()(const grammar::ConceptAtomicStateImpl<Derived>* l,
                                                                                           const grammar::ConceptAtomicStateImpl<Derived>* r) const;

template<PredicateCategory P>
bool UniqueDLEqualTo<const grammar::ConceptAtomicGoalImpl<P>*>::operator()(const grammar::ConceptAtomicGoalImpl<P>* l,
                                                                           const grammar::ConceptAtomicGoalImpl<P>* r) const
{
    if (&l != &r)
    {
        return (l->get_predicate() == r->get_predicate());
    }
    return true;
}

template bool UniqueDLEqualTo<const grammar::ConceptAtomicGoalImpl<Static>*>::operator()(const grammar::ConceptAtomicGoalImpl<Static>* l,
                                                                                         const grammar::ConceptAtomicGoalImpl<Static>* r) const;
template bool UniqueDLEqualTo<const grammar::ConceptAtomicGoalImpl<Fluent>*>::operator()(const grammar::ConceptAtomicGoalImpl<Fluent>* l,
                                                                                         const grammar::ConceptAtomicGoalImpl<Fluent>* r) const;
template bool UniqueDLEqualTo<const grammar::ConceptAtomicGoalImpl<Derived>*>::operator()(const grammar::ConceptAtomicGoalImpl<Derived>* l,
                                                                                          const grammar::ConceptAtomicGoalImpl<Derived>* r) const;

bool UniqueDLEqualTo<const grammar::ConceptIntersectionImpl*>::operator()(const grammar::ConceptIntersectionImpl* l,
                                                                          const grammar::ConceptIntersectionImpl* r) const
{
    if (&l != &r)
    {
        return (l->get_concept_left() == r->get_concept_left()) && (l->get_concept_right() == r->get_concept_right());
    }
    return true;
}

/* Roles */

template<PredicateCategory P>
bool UniqueDLEqualTo<const grammar::RoleAtomicStateImpl<P>*>::operator()(const grammar::RoleAtomicStateImpl<P>* l,
                                                                         const grammar::RoleAtomicStateImpl<P>* r) const
{
    if (&l != &r)
    {
        return (l->get_predicate() == r->get_predicate());
    }
    return true;
}

template bool UniqueDLEqualTo<const grammar::RoleAtomicStateImpl<Static>*>::operator()(const grammar::RoleAtomicStateImpl<Static>* l,
                                                                                       const grammar::RoleAtomicStateImpl<Static>* r) const;
template bool UniqueDLEqualTo<const grammar::RoleAtomicStateImpl<Fluent>*>::operator()(const grammar::RoleAtomicStateImpl<Fluent>* l,
                                                                                       const grammar::RoleAtomicStateImpl<Fluent>* r) const;
template bool UniqueDLEqualTo<const grammar::RoleAtomicStateImpl<Derived>*>::operator()(const grammar::RoleAtomicStateImpl<Derived>* l,
                                                                                        const grammar::RoleAtomicStateImpl<Derived>* r) const;

template<PredicateCategory P>
bool UniqueDLEqualTo<const grammar::RoleAtomicGoalImpl<P>*>::operator()(const grammar::RoleAtomicGoalImpl<P>* l, const grammar::RoleAtomicGoalImpl<P>* r) const
{
    if (&l != &r)
    {
        return (l->get_predicate() == r->get_predicate());
    }
    return true;
}

template bool UniqueDLEqualTo<const grammar::RoleAtomicGoalImpl<Static>*>::operator()(const grammar::RoleAtomicGoalImpl<Static>* l,
                                                                                      const grammar::RoleAtomicGoalImpl<Static>* r) const;
template bool UniqueDLEqualTo<const grammar::RoleAtomicGoalImpl<Fluent>*>::operator()(const grammar::RoleAtomicGoalImpl<Fluent>* l,
                                                                                      const grammar::RoleAtomicGoalImpl<Fluent>* r) const;
template bool UniqueDLEqualTo<const grammar::RoleAtomicGoalImpl<Derived>*>::operator()(const grammar::RoleAtomicGoalImpl<Derived>* l,
                                                                                       const grammar::RoleAtomicGoalImpl<Derived>* r) const;

bool UniqueDLEqualTo<const grammar::RoleIntersectionImpl*>::operator()(const grammar::RoleIntersectionImpl* l, const grammar::RoleIntersectionImpl* r) const
{
    if (&l != &r)
    {
        return (l->get_role_left() == r->get_role_left()) && (l->get_role_right() == r->get_role_right());
    }
    return true;
}

}
