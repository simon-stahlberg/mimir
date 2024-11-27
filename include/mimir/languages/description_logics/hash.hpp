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
#include <ranges>
#include <utility>
#include <variant>

namespace mimir::dl
{

/// @brief `UniqueDLHasher` is used to compare newly created dl constructors for uniqueness.
/// Since the children are unique, it suffices to create a combined hash from nested pointers.
template<typename T>
struct UniqueDLHasher
{
    size_t operator()(const T& element) const { return std::hash<T>()(element); }
};

struct UniqueDLHashCombiner
{
public:
    template<typename T>
    void operator()(size_t& seed, const T& value) const
    {
        seed ^= UniqueDLHasher<T>()(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }

    void operator()(size_t& seed, const std::size_t& value) const { seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2); }

    template<typename... Types>
    size_t operator()(const Types&... args) const
    {
        size_t seed = 0;
        ((*this)(seed, args), ...);
        return seed;
    }
};

/// Spezialization for std::ranges::forward_range.
template<typename ForwardRange>
requires std::ranges::forward_range<ForwardRange>
struct UniqueDLHasher<ForwardRange>
{
    size_t operator()(const ForwardRange& range) const
    {
        std::size_t aggregated_hash = 0;
        for (const auto& item : range)
        {
            UniqueDLHashCombiner()(aggregated_hash, item);
        }
        return aggregated_hash;
    }
};

/// Spezialization for std::variant.
template<typename... Ts>
struct UniqueDLHasher<std::variant<Ts...>>
{
    size_t operator()(const std::variant<Ts...>& variant) const
    {
        return std::visit([](const auto& arg) { return UniqueDLHasher<decltype(arg)>()(arg); }, variant);
    }
};

/**
 * DL constructors
 */

/* Concepts */

template<>
struct UniqueDLHasher<ConceptBot>
{
    size_t operator()(ConceptBot e) const;
};

template<>
struct UniqueDLHasher<ConceptTop>
{
    size_t operator()(ConceptTop e) const;
};

template<PredicateTag P>
struct UniqueDLHasher<ConceptAtomicState<P>>
{
    size_t operator()(ConceptAtomicState<P> e) const;
};

template<PredicateTag P>
struct UniqueDLHasher<ConceptAtomicGoal<P>>
{
    size_t operator()(ConceptAtomicGoal<P> e) const;
};

template<>
struct UniqueDLHasher<ConceptIntersection>
{
    size_t operator()(ConceptIntersection e) const;
};

template<>
struct UniqueDLHasher<ConceptUnion>
{
    size_t operator()(ConceptUnion e) const;
};

template<>
struct UniqueDLHasher<ConceptNegation>
{
    size_t operator()(ConceptNegation e) const;
};

template<>
struct UniqueDLHasher<ConceptValueRestriction>
{
    size_t operator()(ConceptValueRestriction e) const;
};

template<>
struct UniqueDLHasher<ConceptExistentialQuantification>
{
    size_t operator()(ConceptExistentialQuantification e) const;
};

template<>
struct UniqueDLHasher<ConceptRoleValueMapContainment>
{
    size_t operator()(ConceptRoleValueMapContainment e) const;
};

template<>
struct UniqueDLHasher<ConceptRoleValueMapEquality>
{
    size_t operator()(ConceptRoleValueMapEquality e) const;
};

template<>
struct UniqueDLHasher<ConceptNominal>
{
    size_t operator()(ConceptNominal e) const;
};

/* Roles */

template<>
struct UniqueDLHasher<RoleUniversal>
{
    size_t operator()(RoleUniversal e) const;
};

template<PredicateTag P>
struct UniqueDLHasher<RoleAtomicState<P>>
{
    size_t operator()(RoleAtomicState<P> e) const;
};

template<PredicateTag P>
struct UniqueDLHasher<RoleAtomicGoal<P>>
{
    size_t operator()(RoleAtomicGoal<P> e) const;
};

template<>
struct UniqueDLHasher<RoleIntersection>
{
    size_t operator()(RoleIntersection e) const;
};

template<>
struct UniqueDLHasher<RoleUnion>
{
    size_t operator()(RoleUnion e) const;
};

template<>
struct UniqueDLHasher<RoleComplement>
{
    size_t operator()(RoleComplement e) const;
};

template<>
struct UniqueDLHasher<RoleInverse>
{
    size_t operator()(RoleInverse e) const;
};

template<>
struct UniqueDLHasher<RoleComposition>
{
    size_t operator()(RoleComposition e) const;
};

template<>
struct UniqueDLHasher<RoleTransitiveClosure>
{
    size_t operator()(RoleTransitiveClosure e) const;
};

template<>
struct UniqueDLHasher<RoleReflexiveTransitiveClosure>
{
    size_t operator()(RoleReflexiveTransitiveClosure e) const;
};

template<>
struct UniqueDLHasher<RoleRestriction>
{
    size_t operator()(RoleRestriction e) const;
};

template<>
struct UniqueDLHasher<RoleIdentity>
{
    size_t operator()(RoleIdentity e) const;
};

/**
 * DL grammar constructors
 */

template<ConstructorTag D>
struct UniqueDLHasher<grammar::DerivationRule<D>>
{
    size_t operator()(grammar::DerivationRule<D> e) const;
};

template<ConstructorTag D>
struct UniqueDLHasher<grammar::NonTerminal<D>>
{
    size_t operator()(grammar::NonTerminal<D> e) const;
};

template<ConstructorTag D>
struct UniqueDLHasher<grammar::ConstructorOrNonTerminal<D>>
{
    size_t operator()(grammar::ConstructorOrNonTerminal<D> e) const;
};

/* Concepts */

template<>
struct UniqueDLHasher<grammar::ConceptBot>
{
    size_t operator()(grammar::ConceptBot e) const;
};

template<>
struct UniqueDLHasher<grammar::ConceptTop>
{
    size_t operator()(grammar::ConceptTop e) const;
};

template<PredicateTag P>
struct UniqueDLHasher<grammar::ConceptAtomicState<P>>
{
    size_t operator()(grammar::ConceptAtomicState<P> e) const;
};

template<PredicateTag P>
struct UniqueDLHasher<grammar::ConceptAtomicGoal<P>>
{
    size_t operator()(grammar::ConceptAtomicGoal<P> e) const;
};

template<>
struct UniqueDLHasher<grammar::ConceptIntersection>
{
    size_t operator()(grammar::ConceptIntersection e) const;
};

template<>
struct UniqueDLHasher<grammar::ConceptUnion>
{
    size_t operator()(grammar::ConceptUnion e) const;
};

template<>
struct UniqueDLHasher<grammar::ConceptNegation>
{
    size_t operator()(grammar::ConceptNegation e) const;
};

template<>
struct UniqueDLHasher<grammar::ConceptValueRestriction>
{
    size_t operator()(grammar::ConceptValueRestriction e) const;
};

template<>
struct UniqueDLHasher<grammar::ConceptExistentialQuantification>
{
    size_t operator()(grammar::ConceptExistentialQuantification e) const;
};

template<>
struct UniqueDLHasher<grammar::ConceptRoleValueMapContainment>
{
    size_t operator()(grammar::ConceptRoleValueMapContainment e) const;
};

template<>
struct UniqueDLHasher<grammar::ConceptRoleValueMapEquality>
{
    size_t operator()(grammar::ConceptRoleValueMapEquality e) const;
};

template<>
struct UniqueDLHasher<grammar::ConceptNominal>
{
    size_t operator()(grammar::ConceptNominal e) const;
};

/* Roles */

template<>
struct UniqueDLHasher<grammar::RoleUniversal>
{
    size_t operator()(grammar::RoleUniversal e) const;
};

template<PredicateTag P>
struct UniqueDLHasher<grammar::RoleAtomicState<P>>
{
    size_t operator()(grammar::RoleAtomicState<P> e) const;
};

template<PredicateTag P>
struct UniqueDLHasher<grammar::RoleAtomicGoal<P>>
{
    size_t operator()(grammar::RoleAtomicGoal<P> e) const;
};

template<>
struct UniqueDLHasher<grammar::RoleIntersection>
{
    size_t operator()(grammar::RoleIntersection e) const;
};

template<>
struct UniqueDLHasher<grammar::RoleUnion>
{
    size_t operator()(grammar::RoleUnion e) const;
};

template<>
struct UniqueDLHasher<grammar::RoleComplement>
{
    size_t operator()(grammar::RoleComplement e) const;
};

template<>
struct UniqueDLHasher<grammar::RoleInverse>
{
    size_t operator()(grammar::RoleInverse e) const;
};

template<>
struct UniqueDLHasher<grammar::RoleComposition>
{
    size_t operator()(grammar::RoleComposition e) const;
};

template<>
struct UniqueDLHasher<grammar::RoleTransitiveClosure>
{
    size_t operator()(grammar::RoleTransitiveClosure e) const;
};

template<>
struct UniqueDLHasher<grammar::RoleReflexiveTransitiveClosure>
{
    size_t operator()(grammar::RoleReflexiveTransitiveClosure e) const;
};

template<>
struct UniqueDLHasher<grammar::RoleRestriction>
{
    size_t operator()(grammar::RoleRestriction e) const;
};

template<>
struct UniqueDLHasher<grammar::RoleIdentity>
{
    size_t operator()(grammar::RoleIdentity e) const;
};

}

#endif
