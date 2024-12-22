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

#ifndef MIMIR_LANGUAGES_DESCRIPTION_LOGICS_EQUAL_TO_HPP_
#define MIMIR_LANGUAGES_DESCRIPTION_LOGICS_EQUAL_TO_HPP_

#include "mimir/languages/description_logics/declarations.hpp"

#include <functional>
#include <loki/details/utils/observer_ptr.hpp>
#include <variant>

/**
 * DL constructors
 */

/* Concepts */

template<>
struct std::equal_to<loki::ObserverPtr<const mimir::dl::ConceptBotImpl>>
{
    bool operator()(loki::ObserverPtr<const mimir::dl::ConceptBotImpl> lhs, loki::ObserverPtr<const mimir::dl::ConceptBotImpl> rhs) const;
};

template<>
struct std::equal_to<loki::ObserverPtr<const mimir::dl::ConceptTopImpl>>
{
    bool operator()(loki::ObserverPtr<const mimir::dl::ConceptTopImpl> lhs, loki::ObserverPtr<const mimir::dl::ConceptTopImpl> rhs) const;
};

template<mimir::PredicateTag P>
struct std::equal_to<loki::ObserverPtr<const mimir::dl::ConceptAtomicStateImpl<P>>>
{
    bool operator()(loki::ObserverPtr<const mimir::dl::ConceptAtomicStateImpl<P>> lhs, loki::ObserverPtr<const mimir::dl::ConceptAtomicStateImpl<P>> rhs) const;
};

template<mimir::PredicateTag P>
struct std::equal_to<loki::ObserverPtr<const mimir::dl::ConceptAtomicGoalImpl<P>>>
{
    bool operator()(loki::ObserverPtr<const mimir::dl::ConceptAtomicGoalImpl<P>> lhs, loki::ObserverPtr<const mimir::dl::ConceptAtomicGoalImpl<P>> rhs) const;
};

template<>
struct std::equal_to<loki::ObserverPtr<const mimir::dl::ConceptIntersectionImpl>>
{
    bool operator()(loki::ObserverPtr<const mimir::dl::ConceptIntersectionImpl> lhs, loki::ObserverPtr<const mimir::dl::ConceptIntersectionImpl> rhs) const;
};

template<>
struct std::equal_to<loki::ObserverPtr<const mimir::dl::ConceptUnionImpl>>
{
    bool operator()(loki::ObserverPtr<const mimir::dl::ConceptUnionImpl> lhs, loki::ObserverPtr<const mimir::dl::ConceptUnionImpl> rhs) const;
};

template<>
struct std::equal_to<loki::ObserverPtr<const mimir::dl::ConceptNegationImpl>>
{
    bool operator()(loki::ObserverPtr<const mimir::dl::ConceptNegationImpl> lhs, loki::ObserverPtr<const mimir::dl::ConceptNegationImpl> rhs) const;
};

template<>
struct std::equal_to<loki::ObserverPtr<const mimir::dl::ConceptValueRestrictionImpl>>
{
    bool operator()(loki::ObserverPtr<const mimir::dl::ConceptValueRestrictionImpl> lhs,
                    loki::ObserverPtr<const mimir::dl::ConceptValueRestrictionImpl> rhs) const;
};

template<>
struct std::equal_to<loki::ObserverPtr<const mimir::dl::ConceptExistentialQuantificationImpl>>
{
    bool operator()(loki::ObserverPtr<const mimir::dl::ConceptExistentialQuantificationImpl> lhs,
                    loki::ObserverPtr<const mimir::dl::ConceptExistentialQuantificationImpl> rhs) const;
};

template<>
struct std::equal_to<loki::ObserverPtr<const mimir::dl::ConceptRoleValueMapContainmentImpl>>
{
    bool operator()(loki::ObserverPtr<const mimir::dl::ConceptRoleValueMapContainmentImpl> lhs,
                    loki::ObserverPtr<const mimir::dl::ConceptRoleValueMapContainmentImpl> rhs) const;
};

template<>
struct std::equal_to<loki::ObserverPtr<const mimir::dl::ConceptRoleValueMapEqualityImpl>>
{
    bool operator()(loki::ObserverPtr<const mimir::dl::ConceptRoleValueMapEqualityImpl> lhs,
                    loki::ObserverPtr<const mimir::dl::ConceptRoleValueMapEqualityImpl> rhs) const;
};

template<>
struct std::equal_to<loki::ObserverPtr<const mimir::dl::ConceptNominalImpl>>
{
    bool operator()(loki::ObserverPtr<const mimir::dl::ConceptNominalImpl> lhs, loki::ObserverPtr<const mimir::dl::ConceptNominalImpl> rhs) const;
};

/* Roles */

template<>
struct std::equal_to<loki::ObserverPtr<const mimir::dl::RoleUniversalImpl>>
{
    bool operator()(loki::ObserverPtr<const mimir::dl::RoleUniversalImpl> lhs, loki::ObserverPtr<const mimir::dl::RoleUniversalImpl> rhs) const;
};

template<mimir::PredicateTag P>
struct std::equal_to<loki::ObserverPtr<const mimir::dl::RoleAtomicStateImpl<P>>>
{
    bool operator()(loki::ObserverPtr<const mimir::dl::RoleAtomicStateImpl<P>> lhs, loki::ObserverPtr<const mimir::dl::RoleAtomicStateImpl<P>> rhs) const;
};

template<mimir::PredicateTag P>
struct std::equal_to<loki::ObserverPtr<const mimir::dl::RoleAtomicGoalImpl<P>>>
{
    bool operator()(loki::ObserverPtr<const mimir::dl::RoleAtomicGoalImpl<P>> lhs, loki::ObserverPtr<const mimir::dl::RoleAtomicGoalImpl<P>> rhs) const;
};

template<>
struct std::equal_to<loki::ObserverPtr<const mimir::dl::RoleIntersectionImpl>>
{
    bool operator()(loki::ObserverPtr<const mimir::dl::RoleIntersectionImpl> lhs, loki::ObserverPtr<const mimir::dl::RoleIntersectionImpl> rhs) const;
};

template<>
struct std::equal_to<loki::ObserverPtr<const mimir::dl::RoleUnionImpl>>
{
    bool operator()(loki::ObserverPtr<const mimir::dl::RoleUnionImpl> lhs, loki::ObserverPtr<const mimir::dl::RoleUnionImpl> rhs) const;
};

template<>
struct std::equal_to<loki::ObserverPtr<const mimir::dl::RoleComplementImpl>>
{
    bool operator()(loki::ObserverPtr<const mimir::dl::RoleComplementImpl> lhs, loki::ObserverPtr<const mimir::dl::RoleComplementImpl> rhs) const;
};

template<>
struct std::equal_to<loki::ObserverPtr<const mimir::dl::RoleInverseImpl>>
{
    bool operator()(loki::ObserverPtr<const mimir::dl::RoleInverseImpl> lhs, loki::ObserverPtr<const mimir::dl::RoleInverseImpl> rhs) const;
};

template<>
struct std::equal_to<loki::ObserverPtr<const mimir::dl::RoleCompositionImpl>>
{
    bool operator()(loki::ObserverPtr<const mimir::dl::RoleCompositionImpl> lhs, loki::ObserverPtr<const mimir::dl::RoleCompositionImpl> rhs) const;
};

template<>
struct std::equal_to<loki::ObserverPtr<const mimir::dl::RoleTransitiveClosureImpl>>
{
    bool operator()(loki::ObserverPtr<const mimir::dl::RoleTransitiveClosureImpl> lhs, loki::ObserverPtr<const mimir::dl::RoleTransitiveClosureImpl> rhs) const;
};

template<>
struct std::equal_to<loki::ObserverPtr<const mimir::dl::RoleReflexiveTransitiveClosureImpl>>
{
    bool operator()(loki::ObserverPtr<const mimir::dl::RoleReflexiveTransitiveClosureImpl> lhs,
                    loki::ObserverPtr<const mimir::dl::RoleReflexiveTransitiveClosureImpl> rhs) const;
};

template<>
struct std::equal_to<loki::ObserverPtr<const mimir::dl::RoleRestrictionImpl>>
{
    bool operator()(loki::ObserverPtr<const mimir::dl::RoleRestrictionImpl> lhs, loki::ObserverPtr<const mimir::dl::RoleRestrictionImpl> rhs) const;
};

template<>
struct std::equal_to<loki::ObserverPtr<const mimir::dl::RoleIdentityImpl>>
{
    bool operator()(loki::ObserverPtr<const mimir::dl::RoleIdentityImpl> lhs, loki::ObserverPtr<const mimir::dl::RoleIdentityImpl> rhs) const;
};

/**
 * DL grammar constructors
 */

template<mimir::dl::ConstructorTag D>
struct std::equal_to<loki::ObserverPtr<const mimir::dl::grammar::DerivationRuleImpl<D>>>
{
    bool operator()(loki::ObserverPtr<const mimir::dl::grammar::DerivationRuleImpl<D>> lhs,
                    loki::ObserverPtr<const mimir::dl::grammar::DerivationRuleImpl<D>> rhs) const;
};

template<mimir::dl::ConstructorTag D>
struct std::equal_to<loki::ObserverPtr<const mimir::dl::grammar::NonTerminalImpl<D>>>
{
    bool operator()(loki::ObserverPtr<const mimir::dl::grammar::NonTerminalImpl<D>> lhs,
                    loki::ObserverPtr<const mimir::dl::grammar::NonTerminalImpl<D>> rhs) const;
};

template<mimir::dl::ConstructorTag D>
struct std::equal_to<loki::ObserverPtr<const mimir::dl::grammar::ConstructorOrNonTerminalImpl<D>>>
{
    bool operator()(loki::ObserverPtr<const mimir::dl::grammar::ConstructorOrNonTerminalImpl<D>> lhs,
                    loki::ObserverPtr<const mimir::dl::grammar::ConstructorOrNonTerminalImpl<D>> rhs) const;
};

/* Concepts */

template<>
struct std::equal_to<loki::ObserverPtr<const mimir::dl::grammar::ConceptBotImpl>>
{
    bool operator()(loki::ObserverPtr<const mimir::dl::grammar::ConceptBotImpl> lhs, loki::ObserverPtr<const mimir::dl::grammar::ConceptBotImpl> rhs) const;
};

template<>
struct std::equal_to<loki::ObserverPtr<const mimir::dl::grammar::ConceptTopImpl>>
{
    bool operator()(loki::ObserverPtr<const mimir::dl::grammar::ConceptTopImpl> lhs, loki::ObserverPtr<const mimir::dl::grammar::ConceptTopImpl> rhs) const;
};

template<mimir::PredicateTag P>
struct std::equal_to<loki::ObserverPtr<const mimir::dl::grammar::ConceptAtomicStateImpl<P>>>
{
    bool operator()(loki::ObserverPtr<const mimir::dl::grammar::ConceptAtomicStateImpl<P>> lhs,
                    loki::ObserverPtr<const mimir::dl::grammar::ConceptAtomicStateImpl<P>> rhs) const;
};

template<mimir::PredicateTag P>
struct std::equal_to<loki::ObserverPtr<const mimir::dl::grammar::ConceptAtomicGoalImpl<P>>>
{
    bool operator()(loki::ObserverPtr<const mimir::dl::grammar::ConceptAtomicGoalImpl<P>> lhs,
                    loki::ObserverPtr<const mimir::dl::grammar::ConceptAtomicGoalImpl<P>> rhs) const;
};

template<>
struct std::equal_to<loki::ObserverPtr<const mimir::dl::grammar::ConceptIntersectionImpl>>
{
    bool operator()(loki::ObserverPtr<const mimir::dl::grammar::ConceptIntersectionImpl> lhs,
                    loki::ObserverPtr<const mimir::dl::grammar::ConceptIntersectionImpl> rhs) const;
};

template<>
struct std::equal_to<loki::ObserverPtr<const mimir::dl::grammar::ConceptUnionImpl>>
{
    bool operator()(loki::ObserverPtr<const mimir::dl::grammar::ConceptUnionImpl> lhs, loki::ObserverPtr<const mimir::dl::grammar::ConceptUnionImpl> rhs) const;
};

template<>
struct std::equal_to<loki::ObserverPtr<const mimir::dl::grammar::ConceptNegationImpl>>
{
    bool operator()(loki::ObserverPtr<const mimir::dl::grammar::ConceptNegationImpl> lhs,
                    loki::ObserverPtr<const mimir::dl::grammar::ConceptNegationImpl> rhs) const;
};

template<>
struct std::equal_to<loki::ObserverPtr<const mimir::dl::grammar::ConceptValueRestrictionImpl>>
{
    bool operator()(loki::ObserverPtr<const mimir::dl::grammar::ConceptValueRestrictionImpl> lhs,
                    loki::ObserverPtr<const mimir::dl::grammar::ConceptValueRestrictionImpl> rhs) const;
};

template<>
struct std::equal_to<loki::ObserverPtr<const mimir::dl::grammar::ConceptExistentialQuantificationImpl>>
{
    bool operator()(loki::ObserverPtr<const mimir::dl::grammar::ConceptExistentialQuantificationImpl> lhs,
                    loki::ObserverPtr<const mimir::dl::grammar::ConceptExistentialQuantificationImpl> rhs) const;
};

template<>
struct std::equal_to<loki::ObserverPtr<const mimir::dl::grammar::ConceptRoleValueMapContainmentImpl>>
{
    bool operator()(loki::ObserverPtr<const mimir::dl::grammar::ConceptRoleValueMapContainmentImpl> lhs,
                    loki::ObserverPtr<const mimir::dl::grammar::ConceptRoleValueMapContainmentImpl> rhs) const;
};

template<>
struct std::equal_to<loki::ObserverPtr<const mimir::dl::grammar::ConceptRoleValueMapEqualityImpl>>
{
    bool operator()(loki::ObserverPtr<const mimir::dl::grammar::ConceptRoleValueMapEqualityImpl> lhs,
                    loki::ObserverPtr<const mimir::dl::grammar::ConceptRoleValueMapEqualityImpl> rhs) const;
};

template<>
struct std::equal_to<loki::ObserverPtr<const mimir::dl::grammar::ConceptNominalImpl>>
{
    bool operator()(loki::ObserverPtr<const mimir::dl::grammar::ConceptNominalImpl> lhs,
                    loki::ObserverPtr<const mimir::dl::grammar::ConceptNominalImpl> rhs) const;
};

/* Roles */

template<>
struct std::equal_to<loki::ObserverPtr<const mimir::dl::grammar::RoleUniversalImpl>>
{
    bool operator()(loki::ObserverPtr<const mimir::dl::grammar::RoleUniversalImpl> lhs,
                    loki::ObserverPtr<const mimir::dl::grammar::RoleUniversalImpl> rhs) const;
};

template<mimir::PredicateTag P>
struct std::equal_to<loki::ObserverPtr<const mimir::dl::grammar::RoleAtomicStateImpl<P>>>
{
    bool operator()(loki::ObserverPtr<const mimir::dl::grammar::RoleAtomicStateImpl<P>> lhs,
                    loki::ObserverPtr<const mimir::dl::grammar::RoleAtomicStateImpl<P>> rhs) const;
};

template<mimir::PredicateTag P>
struct std::equal_to<loki::ObserverPtr<const mimir::dl::grammar::RoleAtomicGoalImpl<P>>>
{
    bool operator()(loki::ObserverPtr<const mimir::dl::grammar::RoleAtomicGoalImpl<P>> lhs,
                    loki::ObserverPtr<const mimir::dl::grammar::RoleAtomicGoalImpl<P>> rhs) const;
};

template<>
struct std::equal_to<loki::ObserverPtr<const mimir::dl::grammar::RoleIntersectionImpl>>
{
    bool operator()(loki::ObserverPtr<const mimir::dl::grammar::RoleIntersectionImpl> lhs,
                    loki::ObserverPtr<const mimir::dl::grammar::RoleIntersectionImpl> rhs) const;
};

template<>
struct std::equal_to<loki::ObserverPtr<const mimir::dl::grammar::RoleUnionImpl>>
{
    bool operator()(loki::ObserverPtr<const mimir::dl::grammar::RoleUnionImpl> lhs, loki::ObserverPtr<const mimir::dl::grammar::RoleUnionImpl> rhs) const;
};

template<>
struct std::equal_to<loki::ObserverPtr<const mimir::dl::grammar::RoleComplementImpl>>
{
    bool operator()(loki::ObserverPtr<const mimir::dl::grammar::RoleComplementImpl> lhs,
                    loki::ObserverPtr<const mimir::dl::grammar::RoleComplementImpl> rhs) const;
};

template<>
struct std::equal_to<loki::ObserverPtr<const mimir::dl::grammar::RoleInverseImpl>>
{
    bool operator()(loki::ObserverPtr<const mimir::dl::grammar::RoleInverseImpl> lhs, loki::ObserverPtr<const mimir::dl::grammar::RoleInverseImpl> rhs) const;
};

template<>
struct std::equal_to<loki::ObserverPtr<const mimir::dl::grammar::RoleCompositionImpl>>
{
    bool operator()(loki::ObserverPtr<const mimir::dl::grammar::RoleCompositionImpl> lhs,
                    loki::ObserverPtr<const mimir::dl::grammar::RoleCompositionImpl> rhs) const;
};

template<>
struct std::equal_to<loki::ObserverPtr<const mimir::dl::grammar::RoleTransitiveClosureImpl>>
{
    bool operator()(loki::ObserverPtr<const mimir::dl::grammar::RoleTransitiveClosureImpl> lhs,
                    loki::ObserverPtr<const mimir::dl::grammar::RoleTransitiveClosureImpl> rhs) const;
};

template<>
struct std::equal_to<loki::ObserverPtr<const mimir::dl::grammar::RoleReflexiveTransitiveClosureImpl>>
{
    bool operator()(loki::ObserverPtr<const mimir::dl::grammar::RoleReflexiveTransitiveClosureImpl> lhs,
                    loki::ObserverPtr<const mimir::dl::grammar::RoleReflexiveTransitiveClosureImpl> rhs) const;
};

template<>
struct std::equal_to<loki::ObserverPtr<const mimir::dl::grammar::RoleRestrictionImpl>>
{
    bool operator()(loki::ObserverPtr<const mimir::dl::grammar::RoleRestrictionImpl> lhs,
                    loki::ObserverPtr<const mimir::dl::grammar::RoleRestrictionImpl> rhs) const;
};

template<>
struct std::equal_to<loki::ObserverPtr<const mimir::dl::grammar::RoleIdentityImpl>>
{
    bool operator()(loki::ObserverPtr<const mimir::dl::grammar::RoleIdentityImpl> lhs, loki::ObserverPtr<const mimir::dl::grammar::RoleIdentityImpl> rhs) const;
};

#endif
