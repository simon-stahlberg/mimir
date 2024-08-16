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
bool UniqueDLEqualTo<const ConceptPredicateStateImpl<P>*>::operator()(const ConceptPredicateStateImpl<P>* l, const ConceptPredicateStateImpl<P>* r) const
{
    if (&l != &r)
    {
        return (l->get_predicate() == r->get_predicate());
    }
    return true;
}

template bool UniqueDLEqualTo<const ConceptPredicateStateImpl<Static>*>::operator()(const ConceptPredicateStateImpl<Static>* l,
                                                                                    const ConceptPredicateStateImpl<Static>* r) const;
template bool UniqueDLEqualTo<const ConceptPredicateStateImpl<Fluent>*>::operator()(const ConceptPredicateStateImpl<Fluent>* l,
                                                                                    const ConceptPredicateStateImpl<Fluent>* r) const;
template bool UniqueDLEqualTo<const ConceptPredicateStateImpl<Derived>*>::operator()(const ConceptPredicateStateImpl<Derived>* l,
                                                                                     const ConceptPredicateStateImpl<Derived>* r) const;

template<PredicateCategory P>
bool UniqueDLEqualTo<const ConceptPredicateGoalImpl<P>*>::operator()(const ConceptPredicateGoalImpl<P>* l, const ConceptPredicateGoalImpl<P>* r) const
{
    if (&l != &r)
    {
        return (l->get_predicate() == r->get_predicate());
    }
    return true;
}

template bool UniqueDLEqualTo<const ConceptPredicateGoalImpl<Static>*>::operator()(const ConceptPredicateGoalImpl<Static>* l,
                                                                                   const ConceptPredicateGoalImpl<Static>* r) const;
template bool UniqueDLEqualTo<const ConceptPredicateGoalImpl<Fluent>*>::operator()(const ConceptPredicateGoalImpl<Fluent>* l,
                                                                                   const ConceptPredicateGoalImpl<Fluent>* r) const;
template bool UniqueDLEqualTo<const ConceptPredicateGoalImpl<Derived>*>::operator()(const ConceptPredicateGoalImpl<Derived>* l,
                                                                                    const ConceptPredicateGoalImpl<Derived>* r) const;

bool UniqueDLEqualTo<const ConceptAndImpl*>::operator()(const ConceptAndImpl* l, const ConceptAndImpl* r) const
{
    if (&l != &r)
    {
        return (l->get_concept_left() == r->get_concept_left()) && (l->get_concept_right() == r->get_concept_right());
    }
    return true;
}

/* Roles */

template<PredicateCategory P>
bool UniqueDLEqualTo<const RolePredicateStateImpl<P>*>::operator()(const RolePredicateStateImpl<P>* l, const RolePredicateStateImpl<P>* r) const
{
    if (&l != &r)
    {
        return (l->get_predicate() == r->get_predicate());
    }
    return true;
}

template bool UniqueDLEqualTo<const RolePredicateStateImpl<Static>*>::operator()(const RolePredicateStateImpl<Static>* l,
                                                                                 const RolePredicateStateImpl<Static>* r) const;
template bool UniqueDLEqualTo<const RolePredicateStateImpl<Fluent>*>::operator()(const RolePredicateStateImpl<Fluent>* l,
                                                                                 const RolePredicateStateImpl<Fluent>* r) const;
template bool UniqueDLEqualTo<const RolePredicateStateImpl<Derived>*>::operator()(const RolePredicateStateImpl<Derived>* l,
                                                                                  const RolePredicateStateImpl<Derived>* r) const;

template<PredicateCategory P>
bool UniqueDLEqualTo<const RolePredicateGoalImpl<P>*>::operator()(const RolePredicateGoalImpl<P>* l, const RolePredicateGoalImpl<P>* r) const
{
    if (&l != &r)
    {
        return (l->get_predicate() == r->get_predicate());
    }
    return true;
}

template bool UniqueDLEqualTo<const RolePredicateGoalImpl<Static>*>::operator()(const RolePredicateGoalImpl<Static>* l,
                                                                                const RolePredicateGoalImpl<Static>* r) const;
template bool UniqueDLEqualTo<const RolePredicateGoalImpl<Fluent>*>::operator()(const RolePredicateGoalImpl<Fluent>* l,
                                                                                const RolePredicateGoalImpl<Fluent>* r) const;
template bool UniqueDLEqualTo<const RolePredicateGoalImpl<Derived>*>::operator()(const RolePredicateGoalImpl<Derived>* l,
                                                                                 const RolePredicateGoalImpl<Derived>* r) const;

bool UniqueDLEqualTo<const RoleAndImpl*>::operator()(const RoleAndImpl* l, const RoleAndImpl* r) const
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
bool UniqueDLEqualTo<const grammar::ConceptPredicateStateImpl<P>*>::operator()(const grammar::ConceptPredicateStateImpl<P>* l,
                                                                               const grammar::ConceptPredicateStateImpl<P>* r) const
{
    if (&l != &r)
    {
        return (l->get_predicate() == r->get_predicate());
    }
    return true;
}

template bool UniqueDLEqualTo<const grammar::ConceptPredicateStateImpl<Static>*>::operator()(const grammar::ConceptPredicateStateImpl<Static>* l,
                                                                                             const grammar::ConceptPredicateStateImpl<Static>* r) const;
template bool UniqueDLEqualTo<const grammar::ConceptPredicateStateImpl<Fluent>*>::operator()(const grammar::ConceptPredicateStateImpl<Fluent>* l,
                                                                                             const grammar::ConceptPredicateStateImpl<Fluent>* r) const;
template bool UniqueDLEqualTo<const grammar::ConceptPredicateStateImpl<Derived>*>::operator()(const grammar::ConceptPredicateStateImpl<Derived>* l,
                                                                                              const grammar::ConceptPredicateStateImpl<Derived>* r) const;

template<PredicateCategory P>
bool UniqueDLEqualTo<const grammar::ConceptPredicateGoalImpl<P>*>::operator()(const grammar::ConceptPredicateGoalImpl<P>* l,
                                                                              const grammar::ConceptPredicateGoalImpl<P>* r) const
{
    if (&l != &r)
    {
        return (l->get_predicate() == r->get_predicate());
    }
    return true;
}

template bool UniqueDLEqualTo<const grammar::ConceptPredicateGoalImpl<Static>*>::operator()(const grammar::ConceptPredicateGoalImpl<Static>* l,
                                                                                            const grammar::ConceptPredicateGoalImpl<Static>* r) const;
template bool UniqueDLEqualTo<const grammar::ConceptPredicateGoalImpl<Fluent>*>::operator()(const grammar::ConceptPredicateGoalImpl<Fluent>* l,
                                                                                            const grammar::ConceptPredicateGoalImpl<Fluent>* r) const;
template bool UniqueDLEqualTo<const grammar::ConceptPredicateGoalImpl<Derived>*>::operator()(const grammar::ConceptPredicateGoalImpl<Derived>* l,
                                                                                             const grammar::ConceptPredicateGoalImpl<Derived>* r) const;

bool UniqueDLEqualTo<const grammar::ConceptAndImpl*>::operator()(const grammar::ConceptAndImpl* l, const grammar::ConceptAndImpl* r) const
{
    if (&l != &r)
    {
        return (l->get_concept_left() == r->get_concept_left()) && (l->get_concept_right() == r->get_concept_right());
    }
    return true;
}

/* Roles */

template<PredicateCategory P>
bool UniqueDLEqualTo<const grammar::RolePredicateStateImpl<P>*>::operator()(const grammar::RolePredicateStateImpl<P>* l,
                                                                            const grammar::RolePredicateStateImpl<P>* r) const
{
    if (&l != &r)
    {
        return (l->get_predicate() == r->get_predicate());
    }
    return true;
}

template bool UniqueDLEqualTo<const grammar::RolePredicateStateImpl<Static>*>::operator()(const grammar::RolePredicateStateImpl<Static>* l,
                                                                                          const grammar::RolePredicateStateImpl<Static>* r) const;
template bool UniqueDLEqualTo<const grammar::RolePredicateStateImpl<Fluent>*>::operator()(const grammar::RolePredicateStateImpl<Fluent>* l,
                                                                                          const grammar::RolePredicateStateImpl<Fluent>* r) const;
template bool UniqueDLEqualTo<const grammar::RolePredicateStateImpl<Derived>*>::operator()(const grammar::RolePredicateStateImpl<Derived>* l,
                                                                                           const grammar::RolePredicateStateImpl<Derived>* r) const;

template<PredicateCategory P>
bool UniqueDLEqualTo<const grammar::RolePredicateGoalImpl<P>*>::operator()(const grammar::RolePredicateGoalImpl<P>* l,
                                                                           const grammar::RolePredicateGoalImpl<P>* r) const
{
    if (&l != &r)
    {
        return (l->get_predicate() == r->get_predicate());
    }
    return true;
}

template bool UniqueDLEqualTo<const grammar::RolePredicateGoalImpl<Static>*>::operator()(const grammar::RolePredicateGoalImpl<Static>* l,
                                                                                         const grammar::RolePredicateGoalImpl<Static>* r) const;
template bool UniqueDLEqualTo<const grammar::RolePredicateGoalImpl<Fluent>*>::operator()(const grammar::RolePredicateGoalImpl<Fluent>* l,
                                                                                         const grammar::RolePredicateGoalImpl<Fluent>* r) const;
template bool UniqueDLEqualTo<const grammar::RolePredicateGoalImpl<Derived>*>::operator()(const grammar::RolePredicateGoalImpl<Derived>* l,
                                                                                          const grammar::RolePredicateGoalImpl<Derived>* r) const;

bool UniqueDLEqualTo<const grammar::RoleAndImpl*>::operator()(const grammar::RoleAndImpl* l, const grammar::RoleAndImpl* r) const
{
    if (&l != &r)
    {
        return (l->get_role_left() == r->get_role_left()) && (l->get_role_right() == r->get_role_right());
    }
    return true;
}

}
