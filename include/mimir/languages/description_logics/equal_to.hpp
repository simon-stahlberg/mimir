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

template<>
struct UniqueDLEqualTo<ConceptBot>
{
    bool operator()(ConceptBot l, ConceptBot r) const;
};

template<>
struct UniqueDLEqualTo<ConceptTop>
{
    bool operator()(ConceptTop l, ConceptTop r) const;
};

template<PredicateTag P>
struct UniqueDLEqualTo<ConceptAtomicState<P>>
{
    bool operator()(ConceptAtomicState<P> l, ConceptAtomicState<P> r) const;
};

template<PredicateTag P>
struct UniqueDLEqualTo<ConceptAtomicGoal<P>>
{
    bool operator()(ConceptAtomicGoal<P> l, ConceptAtomicGoal<P> r) const;
};

template<>
struct UniqueDLEqualTo<ConceptIntersection>
{
    bool operator()(ConceptIntersection l, ConceptIntersection r) const;
};

template<>
struct UniqueDLEqualTo<ConceptUnion>
{
    bool operator()(ConceptUnion l, ConceptUnion r) const;
};

template<>
struct UniqueDLEqualTo<ConceptNegation>
{
    bool operator()(ConceptNegation l, ConceptNegation r) const;
};

template<>
struct UniqueDLEqualTo<ConceptValueRestriction>
{
    bool operator()(ConceptValueRestriction l, ConceptValueRestriction r) const;
};

template<>
struct UniqueDLEqualTo<ConceptExistentialQuantification>
{
    bool operator()(ConceptExistentialQuantification l, ConceptExistentialQuantification r) const;
};

template<>
struct UniqueDLEqualTo<ConceptRoleValueMapContainment>
{
    bool operator()(ConceptRoleValueMapContainment l, ConceptRoleValueMapContainment r) const;
};

template<>
struct UniqueDLEqualTo<ConceptRoleValueMapEquality>
{
    bool operator()(ConceptRoleValueMapEquality l, ConceptRoleValueMapEquality r) const;
};

template<>
struct UniqueDLEqualTo<ConceptNominal>
{
    bool operator()(ConceptNominal l, ConceptNominal r) const;
};

/* Roles */

template<>
struct UniqueDLEqualTo<RoleUniversal>
{
    bool operator()(RoleUniversal l, RoleUniversal r) const;
};

template<PredicateTag P>
struct UniqueDLEqualTo<RoleAtomicState<P>>
{
    bool operator()(RoleAtomicState<P> l, RoleAtomicState<P> r) const;
};

template<PredicateTag P>
struct UniqueDLEqualTo<RoleAtomicGoal<P>>
{
    bool operator()(RoleAtomicGoal<P> l, RoleAtomicGoal<P> r) const;
};

template<>
struct UniqueDLEqualTo<RoleIntersection>
{
    bool operator()(RoleIntersection l, RoleIntersection r) const;
};

template<>
struct UniqueDLEqualTo<RoleUnion>
{
    bool operator()(RoleUnion l, RoleUnion r) const;
};

template<>
struct UniqueDLEqualTo<RoleComplement>
{
    bool operator()(RoleComplement l, RoleComplement r) const;
};

template<>
struct UniqueDLEqualTo<RoleInverse>
{
    bool operator()(RoleInverse l, RoleInverse r) const;
};

template<>
struct UniqueDLEqualTo<RoleComposition>
{
    bool operator()(RoleComposition l, RoleComposition r) const;
};

template<>
struct UniqueDLEqualTo<RoleTransitiveClosure>
{
    bool operator()(RoleTransitiveClosure l, RoleTransitiveClosure r) const;
};

template<>
struct UniqueDLEqualTo<RoleReflexiveTransitiveClosure>
{
    bool operator()(RoleReflexiveTransitiveClosure l, RoleReflexiveTransitiveClosure r) const;
};

template<>
struct UniqueDLEqualTo<RoleRestriction>
{
    bool operator()(RoleRestriction l, RoleRestriction r) const;
};

template<>
struct UniqueDLEqualTo<RoleIdentity>
{
    bool operator()(RoleIdentity l, RoleIdentity r) const;
};

/**
 * DL grammar constructors
 */

template<ConstructorTag D>
struct UniqueDLEqualTo<grammar::DerivationRule<D>>
{
    bool operator()(grammar::DerivationRule<D> l, grammar::DerivationRule<D> r) const;
};

template<ConstructorTag D>
struct UniqueDLEqualTo<grammar::NonTerminal<D>>
{
    bool operator()(grammar::NonTerminal<D> l, grammar::NonTerminal<D> r) const;
};

template<ConstructorTag D>
struct UniqueDLEqualTo<grammar::ConstructorOrNonTerminal<D>>
{
    bool operator()(grammar::ConstructorOrNonTerminal<D> l, grammar::ConstructorOrNonTerminal<D> r) const;
};

/* Concepts */

template<>
struct UniqueDLEqualTo<grammar::ConceptBot>
{
    bool operator()(grammar::ConceptBot l, grammar::ConceptBot r) const;
};

template<>
struct UniqueDLEqualTo<grammar::ConceptTop>
{
    bool operator()(grammar::ConceptTop l, grammar::ConceptTop r) const;
};

template<PredicateTag P>
struct UniqueDLEqualTo<grammar::ConceptAtomicState<P>>
{
    bool operator()(grammar::ConceptAtomicState<P> l, grammar::ConceptAtomicState<P> r) const;
};

template<PredicateTag P>
struct UniqueDLEqualTo<grammar::ConceptAtomicGoal<P>>
{
    bool operator()(grammar::ConceptAtomicGoal<P> l, grammar::ConceptAtomicGoal<P> r) const;
};

template<>
struct UniqueDLEqualTo<grammar::ConceptIntersection>
{
    bool operator()(grammar::ConceptIntersection l, grammar::ConceptIntersection r) const;
};

template<>
struct UniqueDLEqualTo<grammar::ConceptUnion>
{
    bool operator()(grammar::ConceptUnion l, grammar::ConceptUnion r) const;
};

template<>
struct UniqueDLEqualTo<grammar::ConceptNegation>
{
    bool operator()(grammar::ConceptNegation l, grammar::ConceptNegation r) const;
};

template<>
struct UniqueDLEqualTo<grammar::ConceptValueRestriction>
{
    bool operator()(grammar::ConceptValueRestriction l, grammar::ConceptValueRestriction r) const;
};

template<>
struct UniqueDLEqualTo<grammar::ConceptExistentialQuantification>
{
    bool operator()(grammar::ConceptExistentialQuantification l, grammar::ConceptExistentialQuantification r) const;
};

template<>
struct UniqueDLEqualTo<grammar::ConceptRoleValueMapContainment>
{
    bool operator()(grammar::ConceptRoleValueMapContainment l, grammar::ConceptRoleValueMapContainment r) const;
};

template<>
struct UniqueDLEqualTo<grammar::ConceptRoleValueMapEquality>
{
    bool operator()(grammar::ConceptRoleValueMapEquality l, grammar::ConceptRoleValueMapEquality r) const;
};

template<>
struct UniqueDLEqualTo<grammar::ConceptNominal>
{
    bool operator()(grammar::ConceptNominal l, grammar::ConceptNominal r) const;
};

/* Roles */

template<>
struct UniqueDLEqualTo<grammar::RoleUniversal>
{
    bool operator()(grammar::RoleUniversal l, grammar::RoleUniversal r) const;
};

template<PredicateTag P>
struct UniqueDLEqualTo<grammar::RoleAtomicState<P>>
{
    bool operator()(grammar::RoleAtomicState<P> l, grammar::RoleAtomicState<P> r) const;
};

template<PredicateTag P>
struct UniqueDLEqualTo<grammar::RoleAtomicGoal<P>>
{
    bool operator()(grammar::RoleAtomicGoal<P> l, grammar::RoleAtomicGoal<P> r) const;
};

template<>
struct UniqueDLEqualTo<grammar::RoleIntersection>
{
    bool operator()(grammar::RoleIntersection l, grammar::RoleIntersection r) const;
};

template<>
struct UniqueDLEqualTo<grammar::RoleUnion>
{
    bool operator()(grammar::RoleUnion l, grammar::RoleUnion r) const;
};

template<>
struct UniqueDLEqualTo<grammar::RoleComplement>
{
    bool operator()(grammar::RoleComplement l, grammar::RoleComplement r) const;
};

template<>
struct UniqueDLEqualTo<grammar::RoleInverse>
{
    bool operator()(grammar::RoleInverse l, grammar::RoleInverse r) const;
};

template<>
struct UniqueDLEqualTo<grammar::RoleComposition>
{
    bool operator()(grammar::RoleComposition l, grammar::RoleComposition r) const;
};

template<>
struct UniqueDLEqualTo<grammar::RoleTransitiveClosure>
{
    bool operator()(grammar::RoleTransitiveClosure l, grammar::RoleTransitiveClosure r) const;
};

template<>
struct UniqueDLEqualTo<grammar::RoleReflexiveTransitiveClosure>
{
    bool operator()(grammar::RoleReflexiveTransitiveClosure l, grammar::RoleReflexiveTransitiveClosure r) const;
};

template<>
struct UniqueDLEqualTo<grammar::RoleRestriction>
{
    bool operator()(grammar::RoleRestriction l, grammar::RoleRestriction r) const;
};

template<>
struct UniqueDLEqualTo<grammar::RoleIdentity>
{
    bool operator()(grammar::RoleIdentity l, grammar::RoleIdentity r) const;
};

}

#endif
