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

bool UniqueDLEqualTo<ConceptBot>::operator()(ConceptBot l, ConceptBot r) const
{
    if (&l != &r)
    {
        return false;
    }
    return true;
}

bool UniqueDLEqualTo<ConceptTop>::operator()(ConceptTop l, ConceptTop r) const
{
    if (&l != &r)
    {
        return false;
    }
    return true;
}

template<PredicateTag P>
bool UniqueDLEqualTo<ConceptAtomicState<P>>::operator()(ConceptAtomicState<P> l, ConceptAtomicState<P> r) const
{
    if (&l != &r)
    {
        return (l->get_predicate() == r->get_predicate());
    }
    return true;
}

template bool UniqueDLEqualTo<ConceptAtomicState<Static>>::operator()(ConceptAtomicState<Static> l, ConceptAtomicState<Static> r) const;
template bool UniqueDLEqualTo<ConceptAtomicState<Fluent>>::operator()(ConceptAtomicState<Fluent> l, ConceptAtomicState<Fluent> r) const;
template bool UniqueDLEqualTo<ConceptAtomicState<Derived>>::operator()(ConceptAtomicState<Derived> l, ConceptAtomicState<Derived> r) const;

template<PredicateTag P>
bool UniqueDLEqualTo<ConceptAtomicGoal<P>>::operator()(ConceptAtomicGoal<P> l, ConceptAtomicGoal<P> r) const
{
    if (&l != &r)
    {
        return (l->is_negated() == r->is_negated()) && (l->get_predicate() == r->get_predicate());
    }
    return true;
}

template bool UniqueDLEqualTo<ConceptAtomicGoal<Static>>::operator()(ConceptAtomicGoal<Static> l, ConceptAtomicGoal<Static> r) const;
template bool UniqueDLEqualTo<ConceptAtomicGoal<Fluent>>::operator()(ConceptAtomicGoal<Fluent> l, ConceptAtomicGoal<Fluent> r) const;
template bool UniqueDLEqualTo<ConceptAtomicGoal<Derived>>::operator()(ConceptAtomicGoal<Derived> l, ConceptAtomicGoal<Derived> r) const;

bool UniqueDLEqualTo<ConceptIntersection>::operator()(ConceptIntersection l, ConceptIntersection r) const
{
    if (&l != &r)
    {
        return (l->get_concept_left() == r->get_concept_left()) && (l->get_concept_right() == r->get_concept_right());
    }
    return true;
}

bool UniqueDLEqualTo<ConceptUnion>::operator()(ConceptUnion l, ConceptUnion r) const
{
    if (&l != &r)
    {
        return (l->get_concept_left() == r->get_concept_left()) && (l->get_concept_right() == r->get_concept_right());
    }
    return true;
}

bool UniqueDLEqualTo<ConceptNegation>::operator()(ConceptNegation l, ConceptNegation r) const
{
    if (&l != &r)
    {
        return (l->get_concept() == r->get_concept());
    }
    return true;
}

bool UniqueDLEqualTo<ConceptValueRestriction>::operator()(ConceptValueRestriction l, ConceptValueRestriction r) const
{
    if (&l != &r)
    {
        return (l->get_role() == r->get_role()) && (l->get_concept() == r->get_concept());
    }
    return true;
}

bool UniqueDLEqualTo<ConceptExistentialQuantification>::operator()(ConceptExistentialQuantification l, ConceptExistentialQuantification r) const
{
    if (&l != &r)
    {
        return (l->get_role() == r->get_role()) && (l->get_concept() == r->get_concept());
    }
    return true;
}

bool UniqueDLEqualTo<ConceptRoleValueMapContainment>::operator()(ConceptRoleValueMapContainment l, ConceptRoleValueMapContainment r) const
{
    if (&l != &r)
    {
        return (l->get_role_left() == r->get_role_left()) && (l->get_role_right() == r->get_role_right());
    }
    return true;
}

bool UniqueDLEqualTo<ConceptRoleValueMapEquality>::operator()(ConceptRoleValueMapEquality l, ConceptRoleValueMapEquality r) const
{
    if (&l != &r)
    {
        return (l->get_role_left() == r->get_role_left()) && (l->get_role_right() == r->get_role_right());
    }
    return true;
}

bool UniqueDLEqualTo<ConceptNominal>::operator()(ConceptNominal l, ConceptNominal r) const
{
    if (&l != &r)
    {
        return (l->get_object() == r->get_object());
    }
    return true;
}

/* Roles */

bool UniqueDLEqualTo<RoleUniversal>::operator()(RoleUniversal l, RoleUniversal r) const
{
    if (&l != &r)
    {
        return false;
    }
    return true;
}

template<PredicateTag P>
bool UniqueDLEqualTo<RoleAtomicState<P>>::operator()(RoleAtomicState<P> l, RoleAtomicState<P> r) const
{
    if (&l != &r)
    {
        return (l->get_predicate() == r->get_predicate());
    }
    return true;
}

template bool UniqueDLEqualTo<RoleAtomicState<Static>>::operator()(RoleAtomicState<Static> l, RoleAtomicState<Static> r) const;
template bool UniqueDLEqualTo<RoleAtomicState<Fluent>>::operator()(RoleAtomicState<Fluent> l, RoleAtomicState<Fluent> r) const;
template bool UniqueDLEqualTo<RoleAtomicState<Derived>>::operator()(RoleAtomicState<Derived> l, RoleAtomicState<Derived> r) const;

template<PredicateTag P>
bool UniqueDLEqualTo<RoleAtomicGoal<P>>::operator()(RoleAtomicGoal<P> l, RoleAtomicGoal<P> r) const
{
    if (&l != &r)
    {
        return (l->is_negated() == r->is_negated()) && (l->get_predicate() == r->get_predicate());
    }
    return true;
}

template bool UniqueDLEqualTo<RoleAtomicGoal<Static>>::operator()(RoleAtomicGoal<Static> l, RoleAtomicGoal<Static> r) const;
template bool UniqueDLEqualTo<RoleAtomicGoal<Fluent>>::operator()(RoleAtomicGoal<Fluent> l, RoleAtomicGoal<Fluent> r) const;
template bool UniqueDLEqualTo<RoleAtomicGoal<Derived>>::operator()(RoleAtomicGoal<Derived> l, RoleAtomicGoal<Derived> r) const;

bool UniqueDLEqualTo<RoleIntersection>::operator()(RoleIntersection l, RoleIntersection r) const
{
    if (&l != &r)
    {
        return (l->get_role_left() == r->get_role_left()) && (l->get_role_right() == r->get_role_right());
    }
    return true;
}

bool UniqueDLEqualTo<RoleUnion>::operator()(RoleUnion l, RoleUnion r) const
{
    if (&l != &r)
    {
        return (l->get_role_left() == r->get_role_left()) && (l->get_role_right() == r->get_role_right());
    }
    return true;
}

bool UniqueDLEqualTo<RoleComplement>::operator()(RoleComplement l, RoleComplement r) const
{
    if (&l != &r)
    {
        return (l->get_role() == r->get_role());
    }
    return true;
}

bool UniqueDLEqualTo<RoleInverse>::operator()(RoleInverse l, RoleInverse r) const
{
    if (&l != &r)
    {
        return (l->get_role() == r->get_role());
    }
    return true;
}

bool UniqueDLEqualTo<RoleComposition>::operator()(RoleComposition l, RoleComposition r) const
{
    if (&l != &r)
    {
        return (l->get_role_left() == r->get_role_left()) && (l->get_role_right() == r->get_role_right());
    }
    return true;
}

bool UniqueDLEqualTo<RoleTransitiveClosure>::operator()(RoleTransitiveClosure l, RoleTransitiveClosure r) const
{
    if (&l != &r)
    {
        return (l->get_role() == r->get_role());
    }
    return true;
}

bool UniqueDLEqualTo<RoleReflexiveTransitiveClosure>::operator()(RoleReflexiveTransitiveClosure l, RoleReflexiveTransitiveClosure r) const
{
    if (&l != &r)
    {
        return (l->get_role() == r->get_role());
    }
    return true;
}

bool UniqueDLEqualTo<RoleRestriction>::operator()(RoleRestriction l, RoleRestriction r) const
{
    if (&l != &r)
    {
        return (l->get_role() == r->get_role()) && (l->get_concept() == r->get_concept());
    }
    return true;
}

bool UniqueDLEqualTo<RoleIdentity>::operator()(RoleIdentity l, RoleIdentity r) const
{
    if (&l != &r)
    {
        return (l->get_concept() == r->get_concept());
    }
    return true;
}

/**
 * DL grammar constructors
 */

template<ConstructorTag D>
bool UniqueDLEqualTo<grammar::DerivationRule<D>>::operator()(grammar::DerivationRule<D> l, grammar::DerivationRule<D> r) const
{
    if (&l != &r)
    {
        return (l->get_non_terminal() == r->get_non_terminal()) && (l->get_constructor_or_non_terminals() == r->get_constructor_or_non_terminals());
    }
    return true;
}

template bool UniqueDLEqualTo<grammar::DerivationRule<Concept>>::operator()(grammar::DerivationRule<Concept> l, grammar::DerivationRule<Concept> r) const;
template bool UniqueDLEqualTo<grammar::DerivationRule<Role>>::operator()(grammar::DerivationRule<Role> l, grammar::DerivationRule<Role> r) const;

template<ConstructorTag D>
bool UniqueDLEqualTo<grammar::NonTerminal<D>>::operator()(grammar::NonTerminal<D> l, grammar::NonTerminal<D> r) const
{
    if (&l != &r)
    {
        return (l->get_name() == r->get_name());
    }
    return true;
}

template bool UniqueDLEqualTo<grammar::NonTerminal<Concept>>::operator()(grammar::NonTerminal<Concept> l, grammar::NonTerminal<Concept> r) const;
template bool UniqueDLEqualTo<grammar::NonTerminal<Role>>::operator()(grammar::NonTerminal<Role> l, grammar::NonTerminal<Role> r) const;

template<ConstructorTag D>
bool UniqueDLEqualTo<grammar::ConstructorOrNonTerminal<D>>::operator()(grammar::ConstructorOrNonTerminal<D> l, grammar::ConstructorOrNonTerminal<D> r) const
{
    if (&l != &r)
    {
        return (l->get_constructor_or_non_terminal() == r->get_constructor_or_non_terminal());
    }
    return true;
}

template bool UniqueDLEqualTo<grammar::ConstructorOrNonTerminal<Concept>>::operator()(grammar::ConstructorOrNonTerminal<Concept> l,
                                                                                      grammar::ConstructorOrNonTerminal<Concept> r) const;
template bool UniqueDLEqualTo<grammar::ConstructorOrNonTerminal<Role>>::operator()(grammar::ConstructorOrNonTerminal<Role> l,
                                                                                   grammar::ConstructorOrNonTerminal<Role> r) const;

/* Concepts */

bool UniqueDLEqualTo<grammar::ConceptBot>::operator()(grammar::ConceptBot l, grammar::ConceptBot r) const
{
    if (&l != &r)
    {
        return false;
    }
    return true;
}

bool UniqueDLEqualTo<grammar::ConceptTop>::operator()(grammar::ConceptTop l, grammar::ConceptTop r) const
{
    if (&l != &r)
    {
        return false;
    }
    return true;
}

template<PredicateTag P>
bool UniqueDLEqualTo<grammar::ConceptAtomicState<P>>::operator()(grammar::ConceptAtomicState<P> l, grammar::ConceptAtomicState<P> r) const
{
    if (&l != &r)
    {
        return (l->get_predicate() == r->get_predicate());
    }
    return true;
}

template bool UniqueDLEqualTo<grammar::ConceptAtomicState<Static>>::operator()(grammar::ConceptAtomicState<Static> l,
                                                                               grammar::ConceptAtomicState<Static> r) const;
template bool UniqueDLEqualTo<grammar::ConceptAtomicState<Fluent>>::operator()(grammar::ConceptAtomicState<Fluent> l,
                                                                               grammar::ConceptAtomicState<Fluent> r) const;
template bool UniqueDLEqualTo<grammar::ConceptAtomicState<Derived>>::operator()(grammar::ConceptAtomicState<Derived> l,
                                                                                grammar::ConceptAtomicState<Derived> r) const;

template<PredicateTag P>
bool UniqueDLEqualTo<grammar::ConceptAtomicGoal<P>>::operator()(grammar::ConceptAtomicGoal<P> l, grammar::ConceptAtomicGoal<P> r) const
{
    if (&l != &r)
    {
        return (l->is_negated() == r->is_negated()) && (l->get_predicate() == r->get_predicate());
    }
    return true;
}

template bool UniqueDLEqualTo<grammar::ConceptAtomicGoal<Static>>::operator()(grammar::ConceptAtomicGoal<Static> l, grammar::ConceptAtomicGoal<Static> r) const;
template bool UniqueDLEqualTo<grammar::ConceptAtomicGoal<Fluent>>::operator()(grammar::ConceptAtomicGoal<Fluent> l, grammar::ConceptAtomicGoal<Fluent> r) const;
template bool UniqueDLEqualTo<grammar::ConceptAtomicGoal<Derived>>::operator()(grammar::ConceptAtomicGoal<Derived> l,
                                                                               grammar::ConceptAtomicGoal<Derived> r) const;

bool UniqueDLEqualTo<grammar::ConceptIntersection>::operator()(grammar::ConceptIntersection l, grammar::ConceptIntersection r) const
{
    if (&l != &r)
    {
        return (l->get_concept_or_non_terminal_left() == r->get_concept_or_non_terminal_left())
               && (l->get_concept_or_non_terminal_right() == r->get_concept_or_non_terminal_right());
    }
    return true;
}

bool UniqueDLEqualTo<grammar::ConceptUnion>::operator()(grammar::ConceptUnion l, grammar::ConceptUnion r) const
{
    if (&l != &r)
    {
        return (l->get_concept_or_non_terminal_left() == r->get_concept_or_non_terminal_left())
               && (l->get_concept_or_non_terminal_right() == r->get_concept_or_non_terminal_right());
    }
    return true;
}

bool UniqueDLEqualTo<grammar::ConceptNegation>::operator()(grammar::ConceptNegation l, grammar::ConceptNegation r) const
{
    if (&l != &r)
    {
        return (l->get_concept_or_non_terminal() == r->get_concept_or_non_terminal());
    }
    return true;
}

bool UniqueDLEqualTo<grammar::ConceptValueRestriction>::operator()(grammar::ConceptValueRestriction l, grammar::ConceptValueRestriction r) const
{
    if (&l != &r)
    {
        return (l->get_role_or_non_terminal() == r->get_role_or_non_terminal()) && (l->get_concept_or_non_terminal() == r->get_concept_or_non_terminal());
    }
    return true;
}

bool UniqueDLEqualTo<grammar::ConceptExistentialQuantification>::operator()(grammar::ConceptExistentialQuantification l,
                                                                            grammar::ConceptExistentialQuantification r) const
{
    if (&l != &r)
    {
        return (l->get_role_or_non_terminal() == r->get_role_or_non_terminal()) && (l->get_concept_or_non_terminal() == r->get_concept_or_non_terminal());
    }
    return true;
}

bool UniqueDLEqualTo<grammar::ConceptRoleValueMapContainment>::operator()(grammar::ConceptRoleValueMapContainment l,
                                                                          grammar::ConceptRoleValueMapContainment r) const
{
    if (&l != &r)
    {
        return (l->get_role_or_non_terminal_left() == r->get_role_or_non_terminal_left())
               && (l->get_role_or_non_terminal_right() == r->get_role_or_non_terminal_right());
    }
    return true;
}

bool UniqueDLEqualTo<grammar::ConceptRoleValueMapEquality>::operator()(grammar::ConceptRoleValueMapEquality l, grammar::ConceptRoleValueMapEquality r) const
{
    if (&l != &r)
    {
        return (l->get_role_or_non_terminal_left() == r->get_role_or_non_terminal_left())
               && (l->get_role_or_non_terminal_right() == r->get_role_or_non_terminal_right());
    }
    return true;
}

bool UniqueDLEqualTo<grammar::ConceptNominal>::operator()(grammar::ConceptNominal l, grammar::ConceptNominal r) const
{
    if (&l != &r)
    {
        return (l->get_object() == r->get_object());
    }
    return true;
}

/* Roles */

bool UniqueDLEqualTo<grammar::RoleUniversal>::operator()(grammar::RoleUniversal l, grammar::RoleUniversal r) const
{
    if (&l != &r)
    {
        return false;
    }
    return true;
}

template<PredicateTag P>
bool UniqueDLEqualTo<grammar::RoleAtomicState<P>>::operator()(grammar::RoleAtomicState<P> l, grammar::RoleAtomicState<P> r) const
{
    if (&l != &r)
    {
        return (l->get_predicate() == r->get_predicate());
    }
    return true;
}

template bool UniqueDLEqualTo<grammar::RoleAtomicState<Static>>::operator()(grammar::RoleAtomicState<Static> l, grammar::RoleAtomicState<Static> r) const;
template bool UniqueDLEqualTo<grammar::RoleAtomicState<Fluent>>::operator()(grammar::RoleAtomicState<Fluent> l, grammar::RoleAtomicState<Fluent> r) const;
template bool UniqueDLEqualTo<grammar::RoleAtomicState<Derived>>::operator()(grammar::RoleAtomicState<Derived> l, grammar::RoleAtomicState<Derived> r) const;

template<PredicateTag P>
bool UniqueDLEqualTo<grammar::RoleAtomicGoal<P>>::operator()(grammar::RoleAtomicGoal<P> l, grammar::RoleAtomicGoal<P> r) const
{
    if (&l != &r)
    {
        return (l->is_negated() == r->is_negated()) && (l->get_predicate() == r->get_predicate());
    }
    return true;
}

template bool UniqueDLEqualTo<grammar::RoleAtomicGoal<Static>>::operator()(grammar::RoleAtomicGoal<Static> l, grammar::RoleAtomicGoal<Static> r) const;
template bool UniqueDLEqualTo<grammar::RoleAtomicGoal<Fluent>>::operator()(grammar::RoleAtomicGoal<Fluent> l, grammar::RoleAtomicGoal<Fluent> r) const;
template bool UniqueDLEqualTo<grammar::RoleAtomicGoal<Derived>>::operator()(grammar::RoleAtomicGoal<Derived> l, grammar::RoleAtomicGoal<Derived> r) const;

bool UniqueDLEqualTo<grammar::RoleIntersection>::operator()(grammar::RoleIntersection l, grammar::RoleIntersection r) const
{
    if (&l != &r)
    {
        return (l->get_role_or_non_terminal_left() == r->get_role_or_non_terminal_left())
               && (l->get_role_or_non_terminal_right() == r->get_role_or_non_terminal_right());
    }
    return true;
}

bool UniqueDLEqualTo<grammar::RoleUnion>::operator()(grammar::RoleUnion l, grammar::RoleUnion r) const
{
    if (&l != &r)
    {
        return (l->get_role_or_non_terminal_left() == r->get_role_or_non_terminal_left())
               && (l->get_role_or_non_terminal_right() == r->get_role_or_non_terminal_right());
    }
    return true;
}

bool UniqueDLEqualTo<grammar::RoleComplement>::operator()(grammar::RoleComplement l, grammar::RoleComplement r) const
{
    if (&l != &r)
    {
        return (l->get_role_or_non_terminal() == r->get_role_or_non_terminal());
    }
    return true;
}

bool UniqueDLEqualTo<grammar::RoleInverse>::operator()(grammar::RoleInverse l, grammar::RoleInverse r) const
{
    if (&l != &r)
    {
        return (l->get_role_or_non_terminal() == r->get_role_or_non_terminal());
    }
    return true;
}

bool UniqueDLEqualTo<grammar::RoleComposition>::operator()(grammar::RoleComposition l, grammar::RoleComposition r) const
{
    if (&l != &r)
    {
        return (l->get_role_or_non_terminal_left() == r->get_role_or_non_terminal_left())
               && (l->get_role_or_non_terminal_right() == r->get_role_or_non_terminal_right());
    }
    return true;
}

bool UniqueDLEqualTo<grammar::RoleTransitiveClosure>::operator()(grammar::RoleTransitiveClosure l, grammar::RoleTransitiveClosure r) const
{
    if (&l != &r)
    {
        return (l->get_role_or_non_terminal() == r->get_role_or_non_terminal());
    }
    return true;
}

bool UniqueDLEqualTo<grammar::RoleReflexiveTransitiveClosure>::operator()(grammar::RoleReflexiveTransitiveClosure l,
                                                                          grammar::RoleReflexiveTransitiveClosure r) const
{
    if (&l != &r)
    {
        return (l->get_role_or_non_terminal() == r->get_role_or_non_terminal());
    }
    return true;
}

bool UniqueDLEqualTo<grammar::RoleRestriction>::operator()(grammar::RoleRestriction l, grammar::RoleRestriction r) const
{
    if (&l != &r)
    {
        return (l->get_role_or_non_terminal() == r->get_role_or_non_terminal()) && (l->get_concept_or_non_terminal() == r->get_concept_or_non_terminal());
    }
    return true;
}

bool UniqueDLEqualTo<grammar::RoleIdentity>::operator()(grammar::RoleIdentity l, grammar::RoleIdentity r) const
{
    if (&l != &r)
    {
        return (l->get_concept_or_non_terminal() == r->get_concept_or_non_terminal());
    }
    return true;
}

}
