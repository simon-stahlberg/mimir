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

#include "mimir/languages/description_logics/hash.hpp"

#include "mimir/common/hash.hpp"
#include "mimir/languages/description_logics/constructors.hpp"
#include "mimir/languages/description_logics/grammar_constructors.hpp"

namespace mimir::dl
{

/**
 * DL constructors
 */

/* Concepts */

size_t UniqueDLHasher<ConceptBot>::operator()(ConceptBot e) const { return mimir::hash_combine(e); }

size_t UniqueDLHasher<ConceptTop>::operator()(ConceptTop e) const { return mimir::hash_combine(e); }

template<PredicateTag P>
size_t UniqueDLHasher<ConceptAtomicState<P>>::operator()(ConceptAtomicState<P> e) const
{
    return mimir::hash_combine(e->get_predicate());
}

template size_t UniqueDLHasher<ConceptAtomicState<Static>>::operator()(ConceptAtomicState<Static> e) const;
template size_t UniqueDLHasher<ConceptAtomicState<Fluent>>::operator()(ConceptAtomicState<Fluent> e) const;
template size_t UniqueDLHasher<ConceptAtomicState<Derived>>::operator()(ConceptAtomicState<Derived> e) const;

template<PredicateTag P>
size_t UniqueDLHasher<ConceptAtomicGoal<P>>::operator()(ConceptAtomicGoal<P> e) const
{
    return mimir::hash_combine(e->is_negated(), e->get_predicate());
}

template size_t UniqueDLHasher<ConceptAtomicGoal<Static>>::operator()(ConceptAtomicGoal<Static> e) const;
template size_t UniqueDLHasher<ConceptAtomicGoal<Fluent>>::operator()(ConceptAtomicGoal<Fluent> e) const;
template size_t UniqueDLHasher<ConceptAtomicGoal<Derived>>::operator()(ConceptAtomicGoal<Derived> e) const;

size_t UniqueDLHasher<ConceptIntersection>::operator()(ConceptIntersection e) const
{
    return mimir::hash_combine(e->get_concept_left(), e->get_concept_right());
}

size_t UniqueDLHasher<ConceptUnion>::operator()(ConceptUnion e) const { return mimir::hash_combine(e->get_concept_left(), e->get_concept_right()); }

size_t UniqueDLHasher<ConceptNegation>::operator()(ConceptNegation e) const { return mimir::hash_combine(e->get_concept()); }

size_t UniqueDLHasher<ConceptValueRestriction>::operator()(ConceptValueRestriction e) const { return mimir::hash_combine(e->get_role(), e->get_concept()); }

size_t UniqueDLHasher<ConceptExistentialQuantification>::operator()(ConceptExistentialQuantification e) const
{
    return mimir::hash_combine(e->get_role(), e->get_concept());
}

size_t UniqueDLHasher<ConceptRoleValueMapContainment>::operator()(ConceptRoleValueMapContainment e) const
{
    return mimir::hash_combine(e->get_role_left(), e->get_role_right());
}

size_t UniqueDLHasher<ConceptRoleValueMapEquality>::operator()(ConceptRoleValueMapEquality e) const
{
    return mimir::hash_combine(e->get_role_left(), e->get_role_right());
}

size_t UniqueDLHasher<ConceptNominal>::operator()(ConceptNominal e) const { return mimir::hash_combine(e->get_object()); }

/* Roles */

size_t UniqueDLHasher<RoleUniversal>::operator()(RoleUniversal e) const { return mimir::hash_combine(e); }

template<PredicateTag P>
size_t UniqueDLHasher<RoleAtomicState<P>>::operator()(RoleAtomicState<P> e) const
{
    return mimir::hash_combine(e->get_predicate());
}

template size_t UniqueDLHasher<RoleAtomicState<Static>>::operator()(RoleAtomicState<Static> e) const;
template size_t UniqueDLHasher<RoleAtomicState<Fluent>>::operator()(RoleAtomicState<Fluent> e) const;
template size_t UniqueDLHasher<RoleAtomicState<Derived>>::operator()(RoleAtomicState<Derived> e) const;

template<PredicateTag P>
size_t UniqueDLHasher<RoleAtomicGoal<P>>::operator()(RoleAtomicGoal<P> e) const
{
    return mimir::hash_combine(e->is_negated(), e->get_predicate());
}

template size_t UniqueDLHasher<RoleAtomicGoal<Static>>::operator()(RoleAtomicGoal<Static> e) const;
template size_t UniqueDLHasher<RoleAtomicGoal<Fluent>>::operator()(RoleAtomicGoal<Fluent> e) const;
template size_t UniqueDLHasher<RoleAtomicGoal<Derived>>::operator()(RoleAtomicGoal<Derived> e) const;

size_t UniqueDLHasher<RoleIntersection>::operator()(RoleIntersection e) const { return mimir::hash_combine(e->get_role_left(), e->get_role_right()); }

size_t UniqueDLHasher<RoleUnion>::operator()(RoleUnion e) const { return mimir::hash_combine(e->get_role_left(), e->get_role_right()); }

size_t UniqueDLHasher<RoleComplement>::operator()(RoleComplement e) const { return mimir::hash_combine(e->get_role()); }

size_t UniqueDLHasher<RoleInverse>::operator()(RoleInverse e) const { return mimir::hash_combine(e->get_role()); }

size_t UniqueDLHasher<RoleComposition>::operator()(RoleComposition e) const { return mimir::hash_combine(e->get_role_left(), e->get_role_right()); }

size_t UniqueDLHasher<RoleTransitiveClosure>::operator()(RoleTransitiveClosure e) const { return mimir::hash_combine(e->get_role()); }

size_t UniqueDLHasher<RoleReflexiveTransitiveClosure>::operator()(RoleReflexiveTransitiveClosure e) const { return mimir::hash_combine(e->get_role()); }

size_t UniqueDLHasher<RoleRestriction>::operator()(RoleRestriction e) const { return mimir::hash_combine(e->get_role(), e->get_concept()); }

size_t UniqueDLHasher<RoleIdentity>::operator()(RoleIdentity e) const { return mimir::hash_combine(e->get_concept()); }

/**
 * DL grammar constructors
 */

template<ConstructorTag D>
size_t UniqueDLHasher<grammar::DerivationRule<D>>::operator()(grammar::DerivationRule<D> e) const
{
    return mimir::hash_combine(e->get_non_terminal(), e->get_constructor_or_non_terminals());
}

template size_t UniqueDLHasher<grammar::DerivationRule<Concept>>::operator()(grammar::DerivationRule<Concept> e) const;
template size_t UniqueDLHasher<grammar::DerivationRule<Role>>::operator()(grammar::DerivationRule<Role> e) const;

template<ConstructorTag D>
size_t UniqueDLHasher<grammar::NonTerminal<D>>::operator()(grammar::NonTerminal<D> e) const
{
    return mimir::hash_combine(e->get_name());
}

template size_t UniqueDLHasher<grammar::NonTerminal<Concept>>::operator()(grammar::NonTerminal<Concept> e) const;
template size_t UniqueDLHasher<grammar::NonTerminal<Role>>::operator()(grammar::NonTerminal<Role> e) const;

template<ConstructorTag D>
size_t UniqueDLHasher<grammar::ConstructorOrNonTerminal<D>>::operator()(grammar::ConstructorOrNonTerminal<D> e) const
{
    return mimir::hash_combine(e->get_constructor_or_non_terminal());
}

template size_t UniqueDLHasher<grammar::ConstructorOrNonTerminal<Concept>>::operator()(grammar::ConstructorOrNonTerminal<Concept> e) const;
template size_t UniqueDLHasher<grammar::ConstructorOrNonTerminal<Role>>::operator()(grammar::ConstructorOrNonTerminal<Role> e) const;

/* Concepts */

size_t UniqueDLHasher<grammar::ConceptBot>::operator()(grammar::ConceptBot e) const { return mimir::hash_combine(e); }

size_t UniqueDLHasher<grammar::ConceptTop>::operator()(grammar::ConceptTop e) const { return mimir::hash_combine(e); }

template<PredicateTag P>
size_t UniqueDLHasher<grammar::ConceptAtomicState<P>>::operator()(grammar::ConceptAtomicState<P> e) const
{
    return mimir::hash_combine(e->get_predicate());
}

template size_t UniqueDLHasher<grammar::ConceptAtomicState<Static>>::operator()(grammar::ConceptAtomicState<Static> e) const;
template size_t UniqueDLHasher<grammar::ConceptAtomicState<Fluent>>::operator()(grammar::ConceptAtomicState<Fluent> e) const;
template size_t UniqueDLHasher<grammar::ConceptAtomicState<Derived>>::operator()(grammar::ConceptAtomicState<Derived> e) const;

template<PredicateTag P>
size_t UniqueDLHasher<grammar::ConceptAtomicGoal<P>>::operator()(grammar::ConceptAtomicGoal<P> e) const
{
    return mimir::hash_combine(e->is_negated(), e->get_predicate());
}

template size_t UniqueDLHasher<grammar::ConceptAtomicGoal<Static>>::operator()(grammar::ConceptAtomicGoal<Static> e) const;
template size_t UniqueDLHasher<grammar::ConceptAtomicGoal<Fluent>>::operator()(grammar::ConceptAtomicGoal<Fluent> e) const;
template size_t UniqueDLHasher<grammar::ConceptAtomicGoal<Derived>>::operator()(grammar::ConceptAtomicGoal<Derived> e) const;

size_t UniqueDLHasher<grammar::ConceptIntersection>::operator()(grammar::ConceptIntersection e) const
{
    return mimir::hash_combine(e->get_concept_or_non_terminal_left(), e->get_concept_or_non_terminal_right());
}

size_t UniqueDLHasher<grammar::ConceptUnion>::operator()(grammar::ConceptUnion e) const
{
    return mimir::hash_combine(e->get_concept_or_non_terminal_left(), e->get_concept_or_non_terminal_right());
}

size_t UniqueDLHasher<grammar::ConceptNegation>::operator()(grammar::ConceptNegation e) const { return mimir::hash_combine(e->get_concept_or_non_terminal()); }

size_t UniqueDLHasher<grammar::ConceptValueRestriction>::operator()(grammar::ConceptValueRestriction e) const
{
    return mimir::hash_combine(e->get_role_or_non_terminal(), e->get_concept_or_non_terminal());
}

size_t UniqueDLHasher<grammar::ConceptExistentialQuantification>::operator()(grammar::ConceptExistentialQuantification e) const
{
    return mimir::hash_combine(e->get_role_or_non_terminal(), e->get_concept_or_non_terminal());
}

size_t UniqueDLHasher<grammar::ConceptRoleValueMapContainment>::operator()(grammar::ConceptRoleValueMapContainment e) const
{
    return mimir::hash_combine(e->get_role_or_non_terminal_left(), e->get_role_or_non_terminal_right());
}

size_t UniqueDLHasher<grammar::ConceptRoleValueMapEquality>::operator()(grammar::ConceptRoleValueMapEquality e) const
{
    return mimir::hash_combine(e->get_role_or_non_terminal_left(), e->get_role_or_non_terminal_right());
}

size_t UniqueDLHasher<grammar::ConceptNominal>::operator()(grammar::ConceptNominal e) const { return mimir::hash_combine(e->get_object()); }

/* Roles */

size_t UniqueDLHasher<grammar::RoleUniversal>::operator()(grammar::RoleUniversal e) const { return mimir::hash_combine(e); }

template<PredicateTag P>
size_t UniqueDLHasher<grammar::RoleAtomicState<P>>::operator()(grammar::RoleAtomicState<P> e) const
{
    return mimir::hash_combine(e->get_predicate());
}

template size_t UniqueDLHasher<grammar::RoleAtomicState<Static>>::operator()(grammar::RoleAtomicState<Static> e) const;
template size_t UniqueDLHasher<grammar::RoleAtomicState<Fluent>>::operator()(grammar::RoleAtomicState<Fluent> e) const;
template size_t UniqueDLHasher<grammar::RoleAtomicState<Derived>>::operator()(grammar::RoleAtomicState<Derived> e) const;

template<PredicateTag P>
size_t UniqueDLHasher<grammar::RoleAtomicGoal<P>>::operator()(grammar::RoleAtomicGoal<P> e) const
{
    return mimir::hash_combine(e->is_negated(), e->get_predicate());
}

template size_t UniqueDLHasher<grammar::RoleAtomicGoal<Static>>::operator()(grammar::RoleAtomicGoal<Static> e) const;
template size_t UniqueDLHasher<grammar::RoleAtomicGoal<Fluent>>::operator()(grammar::RoleAtomicGoal<Fluent> e) const;
template size_t UniqueDLHasher<grammar::RoleAtomicGoal<Derived>>::operator()(grammar::RoleAtomicGoal<Derived> e) const;

size_t UniqueDLHasher<grammar::RoleIntersection>::operator()(grammar::RoleIntersection e) const
{
    return mimir::hash_combine(e->get_role_or_non_terminal_left(), e->get_role_or_non_terminal_right());
}

size_t UniqueDLHasher<grammar::RoleUnion>::operator()(grammar::RoleUnion e) const
{
    return mimir::hash_combine(e->get_role_or_non_terminal_left(), e->get_role_or_non_terminal_right());
}

size_t UniqueDLHasher<grammar::RoleComplement>::operator()(grammar::RoleComplement e) const { return mimir::hash_combine(e->get_role_or_non_terminal()); }

size_t UniqueDLHasher<grammar::RoleInverse>::operator()(grammar::RoleInverse e) const { return mimir::hash_combine(e->get_role_or_non_terminal()); }

size_t UniqueDLHasher<grammar::RoleComposition>::operator()(grammar::RoleComposition e) const
{
    return mimir::hash_combine(e->get_role_or_non_terminal_left(), e->get_role_or_non_terminal_right());
}

size_t UniqueDLHasher<grammar::RoleTransitiveClosure>::operator()(grammar::RoleTransitiveClosure e) const
{
    return mimir::hash_combine(e->get_role_or_non_terminal());
}

size_t UniqueDLHasher<grammar::RoleReflexiveTransitiveClosure>::operator()(grammar::RoleReflexiveTransitiveClosure e) const
{
    return mimir::hash_combine(e->get_role_or_non_terminal());
}

size_t UniqueDLHasher<grammar::RoleRestriction>::operator()(grammar::RoleRestriction e) const
{
    return mimir::hash_combine(e->get_role_or_non_terminal(), e->get_concept_or_non_terminal());
}

size_t UniqueDLHasher<grammar::RoleIdentity>::operator()(grammar::RoleIdentity e) const { return mimir::hash_combine(e->get_concept_or_non_terminal()); }

}
