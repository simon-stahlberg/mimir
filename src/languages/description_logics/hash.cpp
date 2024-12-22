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

/* Concepts */

size_t std::hash<loki::ObserverPtr<const mimir::dl::ConceptBotImpl>>::operator()(loki::ObserverPtr<const mimir::dl::ConceptBotImpl> ptr) const { return 0; }

size_t std::hash<loki::ObserverPtr<const mimir::dl::ConceptTopImpl>>::operator()(loki::ObserverPtr<const mimir::dl::ConceptTopImpl> ptr) const { return 0; }

template<mimir::PredicateTag P>
size_t
std::hash<loki::ObserverPtr<const mimir::dl::ConceptAtomicStateImpl<P>>>::operator()(loki::ObserverPtr<const mimir::dl::ConceptAtomicStateImpl<P>> ptr) const
{
    return mimir::hash_combine(ptr->get_predicate());
}

template struct std::hash<loki::ObserverPtr<const mimir::dl::ConceptAtomicStateImpl<mimir::Static>>>;
template struct std::hash<loki::ObserverPtr<const mimir::dl::ConceptAtomicStateImpl<mimir::Fluent>>>;
template struct std::hash<loki::ObserverPtr<const mimir::dl::ConceptAtomicStateImpl<mimir::Derived>>>;

template<mimir::PredicateTag P>
size_t
std::hash<loki::ObserverPtr<const mimir::dl::ConceptAtomicGoalImpl<P>>>::operator()(loki::ObserverPtr<const mimir::dl::ConceptAtomicGoalImpl<P>> ptr) const
{
    return mimir::hash_combine(ptr->is_negated(), ptr->get_predicate());
}

template struct std::hash<loki::ObserverPtr<const mimir::dl::ConceptAtomicGoalImpl<mimir::Static>>>;
template struct std::hash<loki::ObserverPtr<const mimir::dl::ConceptAtomicGoalImpl<mimir::Fluent>>>;
template struct std::hash<loki::ObserverPtr<const mimir::dl::ConceptAtomicGoalImpl<mimir::Derived>>>;

size_t std::hash<loki::ObserverPtr<const mimir::dl::ConceptIntersectionImpl>>::operator()(loki::ObserverPtr<const mimir::dl::ConceptIntersectionImpl> ptr) const
{
    return mimir::hash_combine(ptr->get_concept_left(), ptr->get_concept_right());
}

size_t std::hash<loki::ObserverPtr<const mimir::dl::ConceptUnionImpl>>::operator()(loki::ObserverPtr<const mimir::dl::ConceptUnionImpl> ptr) const
{
    return mimir::hash_combine(ptr->get_concept_left(), ptr->get_concept_right());
}

size_t std::hash<loki::ObserverPtr<const mimir::dl::ConceptNegationImpl>>::operator()(loki::ObserverPtr<const mimir::dl::ConceptNegationImpl> ptr) const
{
    return mimir::hash_combine(ptr->get_concept());
}

size_t std::hash<loki::ObserverPtr<const mimir::dl::ConceptValueRestrictionImpl>>::operator()(
    loki::ObserverPtr<const mimir::dl::ConceptValueRestrictionImpl> ptr) const
{
    return mimir::hash_combine(ptr->get_role(), ptr->get_concept());
}

size_t std::hash<loki::ObserverPtr<const mimir::dl::ConceptExistentialQuantificationImpl>>::operator()(
    loki::ObserverPtr<const mimir::dl::ConceptExistentialQuantificationImpl> ptr) const
{
    return mimir::hash_combine(ptr->get_role(), ptr->get_concept());
}

size_t std::hash<loki::ObserverPtr<const mimir::dl::ConceptRoleValueMapContainmentImpl>>::operator()(
    loki::ObserverPtr<const mimir::dl::ConceptRoleValueMapContainmentImpl> ptr) const
{
    return mimir::hash_combine(ptr->get_role_left(), ptr->get_role_right());
}

size_t std::hash<loki::ObserverPtr<const mimir::dl::ConceptRoleValueMapEqualityImpl>>::operator()(
    loki::ObserverPtr<const mimir::dl::ConceptRoleValueMapEqualityImpl> ptr) const
{
    return mimir::hash_combine(ptr->get_role_left(), ptr->get_role_right());
}

size_t std::hash<loki::ObserverPtr<const mimir::dl::ConceptNominalImpl>>::operator()(loki::ObserverPtr<const mimir::dl::ConceptNominalImpl> ptr) const
{
    return mimir::hash_combine(ptr->get_object());
}

/* Roles */

size_t std::hash<loki::ObserverPtr<const mimir::dl::RoleUniversalImpl>>::operator()(loki::ObserverPtr<const mimir::dl::RoleUniversalImpl> ptr) const
{
    return 0;
}

template<mimir::PredicateTag P>
size_t std::hash<loki::ObserverPtr<const mimir::dl::RoleAtomicStateImpl<P>>>::operator()(loki::ObserverPtr<const mimir::dl::RoleAtomicStateImpl<P>> ptr) const
{
    return mimir::hash_combine(ptr->get_predicate());
}

template struct std::hash<loki::ObserverPtr<const mimir::dl::RoleAtomicStateImpl<mimir::Static>>>;
template struct std::hash<loki::ObserverPtr<const mimir::dl::RoleAtomicStateImpl<mimir::Fluent>>>;
template struct std::hash<loki::ObserverPtr<const mimir::dl::RoleAtomicStateImpl<mimir::Derived>>>;

template<mimir::PredicateTag P>
size_t std::hash<loki::ObserverPtr<const mimir::dl::RoleAtomicGoalImpl<P>>>::operator()(loki::ObserverPtr<const mimir::dl::RoleAtomicGoalImpl<P>> ptr) const
{
    return mimir::hash_combine(ptr->is_negated(), ptr->get_predicate());
}

template struct std::hash<loki::ObserverPtr<const mimir::dl::RoleAtomicGoalImpl<mimir::Static>>>;
template struct std::hash<loki::ObserverPtr<const mimir::dl::RoleAtomicGoalImpl<mimir::Fluent>>>;
template struct std::hash<loki::ObserverPtr<const mimir::dl::RoleAtomicGoalImpl<mimir::Derived>>>;

size_t std::hash<loki::ObserverPtr<const mimir::dl::RoleIntersectionImpl>>::operator()(loki::ObserverPtr<const mimir::dl::RoleIntersectionImpl> ptr) const
{
    return mimir::hash_combine(ptr->get_role_left(), ptr->get_role_right());
}

size_t std::hash<loki::ObserverPtr<const mimir::dl::RoleUnionImpl>>::operator()(loki::ObserverPtr<const mimir::dl::RoleUnionImpl> ptr) const
{
    return mimir::hash_combine(ptr->get_role_left(), ptr->get_role_right());
}

size_t std::hash<loki::ObserverPtr<const mimir::dl::RoleComplementImpl>>::operator()(loki::ObserverPtr<const mimir::dl::RoleComplementImpl> ptr) const
{
    return mimir::hash_combine(ptr->get_role());
}

size_t std::hash<loki::ObserverPtr<const mimir::dl::RoleInverseImpl>>::operator()(loki::ObserverPtr<const mimir::dl::RoleInverseImpl> ptr) const
{
    return mimir::hash_combine(ptr->get_role());
}

size_t std::hash<loki::ObserverPtr<const mimir::dl::RoleCompositionImpl>>::operator()(loki::ObserverPtr<const mimir::dl::RoleCompositionImpl> ptr) const
{
    return mimir::hash_combine(ptr->get_role_left(), ptr->get_role_right());
}

size_t
std::hash<loki::ObserverPtr<const mimir::dl::RoleTransitiveClosureImpl>>::operator()(loki::ObserverPtr<const mimir::dl::RoleTransitiveClosureImpl> ptr) const
{
    return mimir::hash_combine(ptr->get_role());
}

size_t std::hash<loki::ObserverPtr<const mimir::dl::RoleReflexiveTransitiveClosureImpl>>::operator()(
    loki::ObserverPtr<const mimir::dl::RoleReflexiveTransitiveClosureImpl> ptr) const
{
    return mimir::hash_combine(ptr->get_role());
}

size_t std::hash<loki::ObserverPtr<const mimir::dl::RoleRestrictionImpl>>::operator()(loki::ObserverPtr<const mimir::dl::RoleRestrictionImpl> ptr) const
{
    return mimir::hash_combine(ptr->get_role(), ptr->get_concept());
}

size_t std::hash<loki::ObserverPtr<const mimir::dl::RoleIdentityImpl>>::operator()(loki::ObserverPtr<const mimir::dl::RoleIdentityImpl> ptr) const
{
    return mimir::hash_combine(ptr->get_concept());
}

/* DL Grammar Constructors */

template<mimir::dl::ConstructorTag D>
size_t std::hash<loki::ObserverPtr<const mimir::dl::grammar::DerivationRuleImpl<D>>>::operator()(
    loki::ObserverPtr<const mimir::dl::grammar::DerivationRuleImpl<D>> ptr) const
{
    return mimir::hash_combine(ptr->get_non_terminal(), ptr->get_constructor_or_non_terminals());
}

template struct std::hash<loki::ObserverPtr<const mimir::dl::grammar::DerivationRuleImpl<mimir::dl::Concept>>>;
template struct std::hash<loki::ObserverPtr<const mimir::dl::grammar::DerivationRuleImpl<mimir::dl::Role>>>;

template<mimir::dl::ConstructorTag D>
size_t std::hash<loki::ObserverPtr<const mimir::dl::grammar::NonTerminalImpl<D>>>::operator()(
    loki::ObserverPtr<const mimir::dl::grammar::NonTerminalImpl<D>> ptr) const
{
    return mimir::hash_combine(ptr->get_name());
}

template struct std::hash<loki::ObserverPtr<const mimir::dl::grammar::NonTerminalImpl<mimir::dl::Concept>>>;
template struct std::hash<loki::ObserverPtr<const mimir::dl::grammar::NonTerminalImpl<mimir::dl::Role>>>;

template<mimir::dl::ConstructorTag D>
size_t std::hash<loki::ObserverPtr<const mimir::dl::grammar::ConstructorOrNonTerminalImpl<D>>>::operator()(
    loki::ObserverPtr<const mimir::dl::grammar::ConstructorOrNonTerminalImpl<D>> ptr) const
{
    return mimir::hash_combine(ptr->get_constructor_or_non_terminal());
}

template struct std::hash<loki::ObserverPtr<const mimir::dl::grammar::ConstructorOrNonTerminalImpl<mimir::dl::Concept>>>;
template struct std::hash<loki::ObserverPtr<const mimir::dl::grammar::ConstructorOrNonTerminalImpl<mimir::dl::Role>>>;

/* Concepts */

size_t std::hash<loki::ObserverPtr<const mimir::dl::grammar::ConceptBotImpl>>::operator()(loki::ObserverPtr<const mimir::dl::grammar::ConceptBotImpl> ptr) const
{
    return 0;
}

size_t std::hash<loki::ObserverPtr<const mimir::dl::grammar::ConceptTopImpl>>::operator()(loki::ObserverPtr<const mimir::dl::grammar::ConceptTopImpl> ptr) const
{
    return 0;
}

template<mimir::PredicateTag P>
size_t std::hash<loki::ObserverPtr<const mimir::dl::grammar::ConceptAtomicStateImpl<P>>>::operator()(
    loki::ObserverPtr<const mimir::dl::grammar::ConceptAtomicStateImpl<P>> ptr) const
{
    return mimir::hash_combine(ptr->get_predicate());
}

template struct std::hash<loki::ObserverPtr<const mimir::dl::grammar::ConceptAtomicStateImpl<mimir::Static>>>;
template struct std::hash<loki::ObserverPtr<const mimir::dl::grammar::ConceptAtomicStateImpl<mimir::Fluent>>>;
template struct std::hash<loki::ObserverPtr<const mimir::dl::grammar::ConceptAtomicStateImpl<mimir::Derived>>>;

template<mimir::PredicateTag P>
size_t std::hash<loki::ObserverPtr<const mimir::dl::grammar::ConceptAtomicGoalImpl<P>>>::operator()(
    loki::ObserverPtr<const mimir::dl::grammar::ConceptAtomicGoalImpl<P>> ptr) const
{
    return mimir::hash_combine(ptr->is_negated(), ptr->get_predicate());
}

template struct std::hash<loki::ObserverPtr<const mimir::dl::grammar::ConceptAtomicGoalImpl<mimir::Static>>>;
template struct std::hash<loki::ObserverPtr<const mimir::dl::grammar::ConceptAtomicGoalImpl<mimir::Fluent>>>;
template struct std::hash<loki::ObserverPtr<const mimir::dl::grammar::ConceptAtomicGoalImpl<mimir::Derived>>>;

size_t std::hash<loki::ObserverPtr<const mimir::dl::grammar::ConceptIntersectionImpl>>::operator()(
    loki::ObserverPtr<const mimir::dl::grammar::ConceptIntersectionImpl> ptr) const
{
    return mimir::hash_combine(ptr->get_concept_or_non_terminal_left(), ptr->get_concept_or_non_terminal_right());
}

size_t
std::hash<loki::ObserverPtr<const mimir::dl::grammar::ConceptUnionImpl>>::operator()(loki::ObserverPtr<const mimir::dl::grammar::ConceptUnionImpl> ptr) const
{
    return mimir::hash_combine(ptr->get_concept_or_non_terminal_left(), ptr->get_concept_or_non_terminal_right());
}

size_t std::hash<loki::ObserverPtr<const mimir::dl::grammar::ConceptNegationImpl>>::operator()(
    loki::ObserverPtr<const mimir::dl::grammar::ConceptNegationImpl> ptr) const
{
    return mimir::hash_combine(ptr->get_concept_or_non_terminal());
}

size_t std::hash<loki::ObserverPtr<const mimir::dl::grammar::ConceptValueRestrictionImpl>>::operator()(
    loki::ObserverPtr<const mimir::dl::grammar::ConceptValueRestrictionImpl> ptr) const
{
    return mimir::hash_combine(ptr->get_role_or_non_terminal(), ptr->get_concept_or_non_terminal());
}

size_t std::hash<loki::ObserverPtr<const mimir::dl::grammar::ConceptExistentialQuantificationImpl>>::operator()(
    loki::ObserverPtr<const mimir::dl::grammar::ConceptExistentialQuantificationImpl> ptr) const
{
    return mimir::hash_combine(ptr->get_role_or_non_terminal(), ptr->get_concept_or_non_terminal());
}

size_t std::hash<loki::ObserverPtr<const mimir::dl::grammar::ConceptRoleValueMapContainmentImpl>>::operator()(
    loki::ObserverPtr<const mimir::dl::grammar::ConceptRoleValueMapContainmentImpl> ptr) const
{
    return mimir::hash_combine(ptr->get_role_or_non_terminal_left(), ptr->get_role_or_non_terminal_right());
}

size_t std::hash<loki::ObserverPtr<const mimir::dl::grammar::ConceptRoleValueMapEqualityImpl>>::operator()(
    loki::ObserverPtr<const mimir::dl::grammar::ConceptRoleValueMapEqualityImpl> ptr) const
{
    return mimir::hash_combine(ptr->get_role_or_non_terminal_left(), ptr->get_role_or_non_terminal_right());
}

size_t std::hash<loki::ObserverPtr<const mimir::dl::grammar::ConceptNominalImpl>>::operator()(
    loki::ObserverPtr<const mimir::dl::grammar::ConceptNominalImpl> ptr) const
{
    return mimir::hash_combine(ptr->get_object());
}

/* Roles */

size_t
std::hash<loki::ObserverPtr<const mimir::dl::grammar::RoleUniversalImpl>>::operator()(loki::ObserverPtr<const mimir::dl::grammar::RoleUniversalImpl> ptr) const
{
    return 0;
}

template<mimir::PredicateTag P>
size_t std::hash<loki::ObserverPtr<const mimir::dl::grammar::RoleAtomicStateImpl<P>>>::operator()(
    loki::ObserverPtr<const mimir::dl::grammar::RoleAtomicStateImpl<P>> ptr) const
{
    return mimir::hash_combine(ptr->get_predicate());
}

template struct std::hash<loki::ObserverPtr<const mimir::dl::grammar::RoleAtomicStateImpl<mimir::Static>>>;
template struct std::hash<loki::ObserverPtr<const mimir::dl::grammar::RoleAtomicStateImpl<mimir::Fluent>>>;
template struct std::hash<loki::ObserverPtr<const mimir::dl::grammar::RoleAtomicStateImpl<mimir::Derived>>>;

template<mimir::PredicateTag P>
size_t std::hash<loki::ObserverPtr<const mimir::dl::grammar::RoleAtomicGoalImpl<P>>>::operator()(
    loki::ObserverPtr<const mimir::dl::grammar::RoleAtomicGoalImpl<P>> ptr) const
{
    return mimir::hash_combine(ptr->is_negated(), ptr->get_predicate());
}

template struct std::hash<loki::ObserverPtr<const mimir::dl::grammar::RoleAtomicGoalImpl<mimir::Static>>>;
template struct std::hash<loki::ObserverPtr<const mimir::dl::grammar::RoleAtomicGoalImpl<mimir::Fluent>>>;
template struct std::hash<loki::ObserverPtr<const mimir::dl::grammar::RoleAtomicGoalImpl<mimir::Derived>>>;

size_t std::hash<loki::ObserverPtr<const mimir::dl::grammar::RoleIntersectionImpl>>::operator()(
    loki::ObserverPtr<const mimir::dl::grammar::RoleIntersectionImpl> ptr) const
{
    return mimir::hash_combine(ptr->get_role_or_non_terminal_left(), ptr->get_role_or_non_terminal_right());
}

size_t std::hash<loki::ObserverPtr<const mimir::dl::grammar::RoleUnionImpl>>::operator()(loki::ObserverPtr<const mimir::dl::grammar::RoleUnionImpl> ptr) const
{
    return mimir::hash_combine(ptr->get_role_or_non_terminal_left(), ptr->get_role_or_non_terminal_right());
}

size_t std::hash<loki::ObserverPtr<const mimir::dl::grammar::RoleComplementImpl>>::operator()(
    loki::ObserverPtr<const mimir::dl::grammar::RoleComplementImpl> ptr) const
{
    return mimir::hash_combine(ptr->get_role_or_non_terminal());
}

size_t
std::hash<loki::ObserverPtr<const mimir::dl::grammar::RoleInverseImpl>>::operator()(loki::ObserverPtr<const mimir::dl::grammar::RoleInverseImpl> ptr) const
{
    return mimir::hash_combine(ptr->get_role_or_non_terminal());
}

size_t std::hash<loki::ObserverPtr<const mimir::dl::grammar::RoleCompositionImpl>>::operator()(
    loki::ObserverPtr<const mimir::dl::grammar::RoleCompositionImpl> ptr) const
{
    return mimir::hash_combine(ptr->get_role_or_non_terminal_left(), ptr->get_role_or_non_terminal_right());
}

size_t std::hash<loki::ObserverPtr<const mimir::dl::grammar::RoleTransitiveClosureImpl>>::operator()(
    loki::ObserverPtr<const mimir::dl::grammar::RoleTransitiveClosureImpl> ptr) const
{
    return mimir::hash_combine(ptr->get_role_or_non_terminal());
}

size_t std::hash<loki::ObserverPtr<const mimir::dl::grammar::RoleReflexiveTransitiveClosureImpl>>::operator()(
    loki::ObserverPtr<const mimir::dl::grammar::RoleReflexiveTransitiveClosureImpl> ptr) const
{
    return mimir::hash_combine(ptr->get_role_or_non_terminal());
}

size_t std::hash<loki::ObserverPtr<const mimir::dl::grammar::RoleRestrictionImpl>>::operator()(
    loki::ObserverPtr<const mimir::dl::grammar::RoleRestrictionImpl> ptr) const
{
    return mimir::hash_combine(ptr->get_role_or_non_terminal(), ptr->get_concept_or_non_terminal());
}

size_t
std::hash<loki::ObserverPtr<const mimir::dl::grammar::RoleIdentityImpl>>::operator()(loki::ObserverPtr<const mimir::dl::grammar::RoleIdentityImpl> ptr) const
{
    return mimir::hash_combine(ptr->get_concept_or_non_terminal());
}
