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

#ifndef MIMIR_LANGUAGES_DESCRIPTION_LOGICS_HASH_HPP_
#define MIMIR_LANGUAGES_DESCRIPTION_LOGICS_HASH_HPP_

#include "mimir/languages/description_logics/declarations.hpp"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <loki/details/utils/observer_ptr.hpp>
#include <ranges>
#include <utility>
#include <variant>

/* Concepts */

template<>
struct std::hash<loki::ObserverPtr<const mimir::dl::ConceptBotImpl>>
{
    size_t operator()(loki::ObserverPtr<const mimir::dl::ConceptBotImpl> ptr) const;
};

template<>
struct std::hash<loki::ObserverPtr<const mimir::dl::ConceptTopImpl>>
{
    size_t operator()(loki::ObserverPtr<const mimir::dl::ConceptTopImpl> ptr) const;
};

template<mimir::PredicateTag P>
struct std::hash<loki::ObserverPtr<const mimir::dl::ConceptAtomicStateImpl<P>>>
{
    size_t operator()(loki::ObserverPtr<const mimir::dl::ConceptAtomicStateImpl<P>> ptr) const;
};

template<mimir::PredicateTag P>
struct std::hash<loki::ObserverPtr<const mimir::dl::ConceptAtomicGoalImpl<P>>>
{
    size_t operator()(loki::ObserverPtr<const mimir::dl::ConceptAtomicGoalImpl<P>> ptr) const;
};

template<>
struct std::hash<loki::ObserverPtr<const mimir::dl::ConceptIntersectionImpl>>
{
    size_t operator()(loki::ObserverPtr<const mimir::dl::ConceptIntersectionImpl> ptr) const;
};

template<>
struct std::hash<loki::ObserverPtr<const mimir::dl::ConceptUnionImpl>>
{
    size_t operator()(loki::ObserverPtr<const mimir::dl::ConceptUnionImpl> ptr) const;
};

template<>
struct std::hash<loki::ObserverPtr<const mimir::dl::ConceptNegationImpl>>
{
    size_t operator()(loki::ObserverPtr<const mimir::dl::ConceptNegationImpl> ptr) const;
};

template<>
struct std::hash<loki::ObserverPtr<const mimir::dl::ConceptValueRestrictionImpl>>
{
    size_t operator()(loki::ObserverPtr<const mimir::dl::ConceptValueRestrictionImpl> ptr) const;
};

template<>
struct std::hash<loki::ObserverPtr<const mimir::dl::ConceptExistentialQuantificationImpl>>
{
    size_t operator()(loki::ObserverPtr<const mimir::dl::ConceptExistentialQuantificationImpl> ptr) const;
};

template<>
struct std::hash<loki::ObserverPtr<const mimir::dl::ConceptRoleValueMapContainmentImpl>>
{
    size_t operator()(loki::ObserverPtr<const mimir::dl::ConceptRoleValueMapContainmentImpl> ptr) const;
};

template<>
struct std::hash<loki::ObserverPtr<const mimir::dl::ConceptRoleValueMapEqualityImpl>>
{
    size_t operator()(loki::ObserverPtr<const mimir::dl::ConceptRoleValueMapEqualityImpl> ptr) const;
};

template<>
struct std::hash<loki::ObserverPtr<const mimir::dl::ConceptNominalImpl>>
{
    size_t operator()(loki::ObserverPtr<const mimir::dl::ConceptNominalImpl> ptr) const;
};

/* Roles */

template<>
struct std::hash<loki::ObserverPtr<const mimir::dl::RoleUniversalImpl>>
{
    size_t operator()(loki::ObserverPtr<const mimir::dl::RoleUniversalImpl> ptr) const;
};

template<mimir::PredicateTag P>
struct std::hash<loki::ObserverPtr<const mimir::dl::RoleAtomicStateImpl<P>>>
{
    size_t operator()(loki::ObserverPtr<const mimir::dl::RoleAtomicStateImpl<P>> ptr) const;
};

template<mimir::PredicateTag P>
struct std::hash<loki::ObserverPtr<const mimir::dl::RoleAtomicGoalImpl<P>>>
{
    size_t operator()(loki::ObserverPtr<const mimir::dl::RoleAtomicGoalImpl<P>> ptr) const;
};

template<>
struct std::hash<loki::ObserverPtr<const mimir::dl::RoleIntersectionImpl>>
{
    size_t operator()(loki::ObserverPtr<const mimir::dl::RoleIntersectionImpl> ptr) const;
};

template<>
struct std::hash<loki::ObserverPtr<const mimir::dl::RoleUnionImpl>>
{
    size_t operator()(loki::ObserverPtr<const mimir::dl::RoleUnionImpl> ptr) const;
};

template<>
struct std::hash<loki::ObserverPtr<const mimir::dl::RoleComplementImpl>>
{
    size_t operator()(loki::ObserverPtr<const mimir::dl::RoleComplementImpl> ptr) const;
};

template<>
struct std::hash<loki::ObserverPtr<const mimir::dl::RoleInverseImpl>>
{
    size_t operator()(loki::ObserverPtr<const mimir::dl::RoleInverseImpl> ptr) const;
};

template<>
struct std::hash<loki::ObserverPtr<const mimir::dl::RoleCompositionImpl>>
{
    size_t operator()(loki::ObserverPtr<const mimir::dl::RoleCompositionImpl> ptr) const;
};

template<>
struct std::hash<loki::ObserverPtr<const mimir::dl::RoleTransitiveClosureImpl>>
{
    size_t operator()(loki::ObserverPtr<const mimir::dl::RoleTransitiveClosureImpl> ptr) const;
};

template<>
struct std::hash<loki::ObserverPtr<const mimir::dl::RoleReflexiveTransitiveClosureImpl>>
{
    size_t operator()(loki::ObserverPtr<const mimir::dl::RoleReflexiveTransitiveClosureImpl> ptr) const;
};

template<>
struct std::hash<loki::ObserverPtr<const mimir::dl::RoleRestrictionImpl>>
{
    size_t operator()(loki::ObserverPtr<const mimir::dl::RoleRestrictionImpl> ptr) const;
};

template<>
struct std::hash<loki::ObserverPtr<const mimir::dl::RoleIdentityImpl>>
{
    size_t operator()(loki::ObserverPtr<const mimir::dl::RoleIdentityImpl> ptr) const;
};

/**
 * DL grammar constructors
 */

template<mimir::dl::ConstructorTag D>
struct std::hash<loki::ObserverPtr<const mimir::dl::grammar::DerivationRuleImpl<D>>>
{
    size_t operator()(loki::ObserverPtr<const mimir::dl::grammar::DerivationRuleImpl<D>> ptr) const;
};

template<mimir::dl::ConstructorTag D>
struct std::hash<loki::ObserverPtr<const mimir::dl::grammar::NonTerminalImpl<D>>>
{
    size_t operator()(loki::ObserverPtr<const mimir::dl::grammar::NonTerminalImpl<D>> ptr) const;
};

template<mimir::dl::ConstructorTag D>
struct std::hash<loki::ObserverPtr<const mimir::dl::grammar::ConstructorOrNonTerminalImpl<D>>>
{
    size_t operator()(loki::ObserverPtr<const mimir::dl::grammar::ConstructorOrNonTerminalImpl<D>> ptr) const;
};

/* Concepts */

template<>
struct std::hash<loki::ObserverPtr<const mimir::dl::grammar::ConceptBotImpl>>
{
    size_t operator()(loki::ObserverPtr<const mimir::dl::grammar::ConceptBotImpl> ptr) const;
};

template<>
struct std::hash<loki::ObserverPtr<const mimir::dl::grammar::ConceptTopImpl>>
{
    size_t operator()(loki::ObserverPtr<const mimir::dl::grammar::ConceptTopImpl> ptr) const;
};

template<mimir::PredicateTag P>
struct std::hash<loki::ObserverPtr<const mimir::dl::grammar::ConceptAtomicStateImpl<P>>>
{
    size_t operator()(loki::ObserverPtr<const mimir::dl::grammar::ConceptAtomicStateImpl<P>> ptr) const;
};

template<mimir::PredicateTag P>
struct std::hash<loki::ObserverPtr<const mimir::dl::grammar::ConceptAtomicGoalImpl<P>>>
{
    size_t operator()(loki::ObserverPtr<const mimir::dl::grammar::ConceptAtomicGoalImpl<P>> ptr) const;
};

template<>
struct std::hash<loki::ObserverPtr<const mimir::dl::grammar::ConceptIntersectionImpl>>
{
    size_t operator()(loki::ObserverPtr<const mimir::dl::grammar::ConceptIntersectionImpl> ptr) const;
};

template<>
struct std::hash<loki::ObserverPtr<const mimir::dl::grammar::ConceptUnionImpl>>
{
    size_t operator()(loki::ObserverPtr<const mimir::dl::grammar::ConceptUnionImpl> ptr) const;
};

template<>
struct std::hash<loki::ObserverPtr<const mimir::dl::grammar::ConceptNegationImpl>>
{
    size_t operator()(loki::ObserverPtr<const mimir::dl::grammar::ConceptNegationImpl> ptr) const;
};

template<>
struct std::hash<loki::ObserverPtr<const mimir::dl::grammar::ConceptValueRestrictionImpl>>
{
    size_t operator()(loki::ObserverPtr<const mimir::dl::grammar::ConceptValueRestrictionImpl> ptr) const;
};

template<>
struct std::hash<loki::ObserverPtr<const mimir::dl::grammar::ConceptExistentialQuantificationImpl>>
{
    size_t operator()(loki::ObserverPtr<const mimir::dl::grammar::ConceptExistentialQuantificationImpl> ptr) const;
};

template<>
struct std::hash<loki::ObserverPtr<const mimir::dl::grammar::ConceptRoleValueMapContainmentImpl>>
{
    size_t operator()(loki::ObserverPtr<const mimir::dl::grammar::ConceptRoleValueMapContainmentImpl> ptr) const;
};

template<>
struct std::hash<loki::ObserverPtr<const mimir::dl::grammar::ConceptRoleValueMapEqualityImpl>>
{
    size_t operator()(loki::ObserverPtr<const mimir::dl::grammar::ConceptRoleValueMapEqualityImpl> ptr) const;
};

template<>
struct std::hash<loki::ObserverPtr<const mimir::dl::grammar::ConceptNominalImpl>>
{
    size_t operator()(loki::ObserverPtr<const mimir::dl::grammar::ConceptNominalImpl> ptr) const;
};

/* Roles */

template<>
struct std::hash<loki::ObserverPtr<const mimir::dl::grammar::RoleUniversalImpl>>
{
    size_t operator()(loki::ObserverPtr<const mimir::dl::grammar::RoleUniversalImpl> ptr) const;
};

template<mimir::PredicateTag P>
struct std::hash<loki::ObserverPtr<const mimir::dl::grammar::RoleAtomicStateImpl<P>>>
{
    size_t operator()(loki::ObserverPtr<const mimir::dl::grammar::RoleAtomicStateImpl<P>> ptr) const;
};

template<mimir::PredicateTag P>
struct std::hash<loki::ObserverPtr<const mimir::dl::grammar::RoleAtomicGoalImpl<P>>>
{
    size_t operator()(loki::ObserverPtr<const mimir::dl::grammar::RoleAtomicGoalImpl<P>> ptr) const;
};

template<>
struct std::hash<loki::ObserverPtr<const mimir::dl::grammar::RoleIntersectionImpl>>
{
    size_t operator()(loki::ObserverPtr<const mimir::dl::grammar::RoleIntersectionImpl> ptr) const;
};

template<>
struct std::hash<loki::ObserverPtr<const mimir::dl::grammar::RoleUnionImpl>>
{
    size_t operator()(loki::ObserverPtr<const mimir::dl::grammar::RoleUnionImpl> ptr) const;
};

template<>
struct std::hash<loki::ObserverPtr<const mimir::dl::grammar::RoleComplementImpl>>
{
    size_t operator()(loki::ObserverPtr<const mimir::dl::grammar::RoleComplementImpl> ptr) const;
};

template<>
struct std::hash<loki::ObserverPtr<const mimir::dl::grammar::RoleInverseImpl>>
{
    size_t operator()(loki::ObserverPtr<const mimir::dl::grammar::RoleInverseImpl> ptr) const;
};

template<>
struct std::hash<loki::ObserverPtr<const mimir::dl::grammar::RoleCompositionImpl>>
{
    size_t operator()(loki::ObserverPtr<const mimir::dl::grammar::RoleCompositionImpl> ptr) const;
};

template<>
struct std::hash<loki::ObserverPtr<const mimir::dl::grammar::RoleTransitiveClosureImpl>>
{
    size_t operator()(loki::ObserverPtr<const mimir::dl::grammar::RoleTransitiveClosureImpl> ptr) const;
};

template<>
struct std::hash<loki::ObserverPtr<const mimir::dl::grammar::RoleReflexiveTransitiveClosureImpl>>
{
    size_t operator()(loki::ObserverPtr<const mimir::dl::grammar::RoleReflexiveTransitiveClosureImpl> ptr) const;
};

template<>
struct std::hash<loki::ObserverPtr<const mimir::dl::grammar::RoleRestrictionImpl>>
{
    size_t operator()(loki::ObserverPtr<const mimir::dl::grammar::RoleRestrictionImpl> ptr) const;
};

template<>
struct std::hash<loki::ObserverPtr<const mimir::dl::grammar::RoleIdentityImpl>>
{
    size_t operator()(loki::ObserverPtr<const mimir::dl::grammar::RoleIdentityImpl> ptr) const;
};

#endif
