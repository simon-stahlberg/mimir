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
#include <variant>

namespace mimir::dl
{

/// @brief `UniqueDLEqualTo` is used to compare newly created dl constructors for uniqueness.
/// Since the children are unique, it suffices to compare nested pointers.
template<typename T>
struct UniqueDLEqualTo
{
    bool operator()(const T& l, const T& r) const { return std::equal_to<T>()(l, r); }
};

/// Spezialization for std::variant.
template<typename... Ts>
struct UniqueDLEqualTo<std::variant<Ts...>>
{
    bool operator()(const std::variant<Ts...>& l, const std::variant<Ts...>& r) const
    {
        if (l.index() != r.index())
        {
            return false;  // Different types held
        }
        // Compare the held values, but only if they are of the same type
        return std::visit(
            [](const auto& lhs, const auto& rhs) -> bool
            {
                if constexpr (std::is_same_v<decltype(lhs), decltype(rhs)>)
                {
                    return UniqueDLEqualTo<decltype(lhs)>()(lhs, rhs);
                }
                else
                {
                    return false;  // Different types can't be equal
                }
            },
            l,
            r);
    }
};

/**
 * DL constructors
 */

/* Concepts */

template<PredicateCategory P>
struct UniqueDLEqualTo<const ConceptAtomicStateImpl<P>*>
{
    bool operator()(const ConceptAtomicStateImpl<P>* l, const ConceptAtomicStateImpl<P>* r) const;
};

template<PredicateCategory P>
struct UniqueDLEqualTo<const ConceptAtomicGoalImpl<P>*>
{
    bool operator()(const ConceptAtomicGoalImpl<P>* l, const ConceptAtomicGoalImpl<P>* r) const;
};

template<>
struct UniqueDLEqualTo<const ConceptIntersectionImpl*>
{
    bool operator()(const ConceptIntersectionImpl* l, const ConceptIntersectionImpl* r) const;
};

/* Roles */

template<PredicateCategory P>
struct UniqueDLEqualTo<const RoleAtomicStateImpl<P>*>
{
    bool operator()(const RoleAtomicStateImpl<P>* l, const RoleAtomicStateImpl<P>* r) const;
};

template<PredicateCategory P>
struct UniqueDLEqualTo<const RoleAtomicGoalImpl<P>*>
{
    bool operator()(const RoleAtomicGoalImpl<P>* l, const RoleAtomicGoalImpl<P>* r) const;
};

template<>
struct UniqueDLEqualTo<const RoleIntersectionImpl*>
{
    bool operator()(const RoleIntersectionImpl* l, const RoleIntersectionImpl* r) const;
};

/**
 * DL grammar constructors
 */

template<IsConceptOrRole D>
struct UniqueDLEqualTo<const grammar::DerivationRuleImpl<D>*>
{
    bool operator()(const grammar::DerivationRuleImpl<D>* l, const grammar::DerivationRuleImpl<D>* r) const;
};

template<IsConceptOrRole D>
struct UniqueDLEqualTo<const grammar::NonTerminalImpl<D>*>
{
    bool operator()(const grammar::NonTerminalImpl<D>* l, const grammar::NonTerminalImpl<D>* r) const;
};

template<IsConceptOrRole D>
struct UniqueDLEqualTo<const grammar::ConstructorOrNonTerminalChoice<D>*>
{
    bool operator()(const grammar::ConstructorOrNonTerminalChoice<D>* l, const grammar::ConstructorOrNonTerminalChoice<D>* r) const;
};

template<IsConceptOrRole D>
struct UniqueDLEqualTo<const grammar::ChoiceImpl<D>*>
{
    bool operator()(const grammar::ChoiceImpl<D>* l, const grammar::ChoiceImpl<D>* r) const;
};

/* Concepts */

template<>
struct UniqueDLEqualTo<const grammar::ConceptBotImpl*>
{
    bool operator()(const grammar::ConceptBotImpl* l, const grammar::ConceptBotImpl* r) const;
};

template<>
struct UniqueDLEqualTo<const grammar::ConceptTopImpl*>
{
    bool operator()(const grammar::ConceptTopImpl* l, const grammar::ConceptTopImpl* r) const;
};

template<PredicateCategory P>
struct UniqueDLEqualTo<const grammar::ConceptAtomicStateImpl<P>*>
{
    bool operator()(const grammar::ConceptAtomicStateImpl<P>* l, const grammar::ConceptAtomicStateImpl<P>* r) const;
};

template<PredicateCategory P>
struct UniqueDLEqualTo<const grammar::ConceptAtomicGoalImpl<P>*>
{
    bool operator()(const grammar::ConceptAtomicGoalImpl<P>* l, const grammar::ConceptAtomicGoalImpl<P>* r) const;
};

template<>
struct UniqueDLEqualTo<const grammar::ConceptIntersectionImpl*>
{
    bool operator()(const grammar::ConceptIntersectionImpl* l, const grammar::ConceptIntersectionImpl* r) const;
};

template<>
struct UniqueDLEqualTo<const grammar::ConceptUnionImpl*>
{
    bool operator()(const grammar::ConceptUnionImpl* l, const grammar::ConceptUnionImpl* r) const;
};

template<>
struct UniqueDLEqualTo<const grammar::ConceptNegationImpl*>
{
    bool operator()(const grammar::ConceptNegationImpl* l, const grammar::ConceptNegationImpl* r) const;
};

template<>
struct UniqueDLEqualTo<const grammar::ConceptValueRestrictionImpl*>
{
    bool operator()(const grammar::ConceptValueRestrictionImpl* l, const grammar::ConceptValueRestrictionImpl* r) const;
};

template<>
struct UniqueDLEqualTo<const grammar::ConceptExistentialQuantificationImpl*>
{
    bool operator()(const grammar::ConceptExistentialQuantificationImpl* l, const grammar::ConceptExistentialQuantificationImpl* r) const;
};

template<>
struct UniqueDLEqualTo<const grammar::ConceptRoleValueMapContainmentImpl*>
{
    bool operator()(const grammar::ConceptRoleValueMapContainmentImpl* l, const grammar::ConceptRoleValueMapContainmentImpl* r) const;
};

template<>
struct UniqueDLEqualTo<const grammar::ConceptRoleValueMapEqualityImpl*>
{
    bool operator()(const grammar::ConceptRoleValueMapEqualityImpl* l, const grammar::ConceptRoleValueMapEqualityImpl* r) const;
};

template<>
struct UniqueDLEqualTo<const grammar::ConceptNominalImpl*>
{
    bool operator()(const grammar::ConceptNominalImpl* l, const grammar::ConceptNominalImpl* r) const;
};

/* Roles */

template<>
struct UniqueDLEqualTo<const grammar::RoleUniversalImpl*>
{
    bool operator()(const grammar::RoleUniversalImpl* l, const grammar::RoleUniversalImpl* r) const;
};

template<PredicateCategory P>
struct UniqueDLEqualTo<const grammar::RoleAtomicStateImpl<P>*>
{
    bool operator()(const grammar::RoleAtomicStateImpl<P>* l, const grammar::RoleAtomicStateImpl<P>* r) const;
};

template<PredicateCategory P>
struct UniqueDLEqualTo<const grammar::RoleAtomicGoalImpl<P>*>
{
    bool operator()(const grammar::RoleAtomicGoalImpl<P>* l, const grammar::RoleAtomicGoalImpl<P>* r) const;
};

template<>
struct UniqueDLEqualTo<const grammar::RoleIntersectionImpl*>
{
    bool operator()(const grammar::RoleIntersectionImpl* l, const grammar::RoleIntersectionImpl* r) const;
};

template<>
struct UniqueDLEqualTo<const grammar::RoleUnionImpl*>
{
    bool operator()(const grammar::RoleUnionImpl* l, const grammar::RoleUnionImpl* r) const;
};

template<>
struct UniqueDLEqualTo<const grammar::RoleComplementImpl*>
{
    bool operator()(const grammar::RoleComplementImpl* l, const grammar::RoleComplementImpl* r) const;
};

template<>
struct UniqueDLEqualTo<const grammar::RoleInverseImpl*>
{
    bool operator()(const grammar::RoleInverseImpl* l, const grammar::RoleInverseImpl* r) const;
};

template<>
struct UniqueDLEqualTo<const grammar::RoleCompositionImpl*>
{
    bool operator()(const grammar::RoleCompositionImpl* l, const grammar::RoleCompositionImpl* r) const;
};

template<>
struct UniqueDLEqualTo<const grammar::RoleTransitiveClosureImpl*>
{
    bool operator()(const grammar::RoleTransitiveClosureImpl* l, const grammar::RoleTransitiveClosureImpl* r) const;
};

template<>
struct UniqueDLEqualTo<const grammar::RoleReflexiveTransitiveClosureImpl*>
{
    bool operator()(const grammar::RoleReflexiveTransitiveClosureImpl* l, const grammar::RoleReflexiveTransitiveClosureImpl* r) const;
};

template<>
struct UniqueDLEqualTo<const grammar::RoleRestrictionImpl*>
{
    bool operator()(const grammar::RoleRestrictionImpl* l, const grammar::RoleRestrictionImpl* r) const;
};

template<>
struct UniqueDLEqualTo<const grammar::RoleIdentityImpl*>
{
    bool operator()(const grammar::RoleIdentityImpl* l, const grammar::RoleIdentityImpl* r) const;
};

}

#endif
