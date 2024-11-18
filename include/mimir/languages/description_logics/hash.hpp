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
struct UniqueDLHasher<const ConceptBotImpl*>
{
    size_t operator()(const ConceptBotImpl* e) const;
};

template<>
struct UniqueDLHasher<const ConceptTopImpl*>
{
    size_t operator()(const ConceptTopImpl* e) const;
};

template<PredicateTag P>
struct UniqueDLHasher<const ConceptAtomicStateImpl<P>*>
{
    size_t operator()(const ConceptAtomicStateImpl<P>* e) const;
};

template<PredicateTag P>
struct UniqueDLHasher<const ConceptAtomicGoalImpl<P>*>
{
    size_t operator()(const ConceptAtomicGoalImpl<P>* e) const;
};

template<>
struct UniqueDLHasher<const ConceptIntersectionImpl*>
{
    size_t operator()(const ConceptIntersectionImpl* e) const;
};

template<>
struct UniqueDLHasher<const ConceptUnionImpl*>
{
    size_t operator()(const ConceptUnionImpl* e) const;
};

template<>
struct UniqueDLHasher<const ConceptNegationImpl*>
{
    size_t operator()(const ConceptNegationImpl* e) const;
};

template<>
struct UniqueDLHasher<const ConceptValueRestrictionImpl*>
{
    size_t operator()(const ConceptValueRestrictionImpl* e) const;
};

template<>
struct UniqueDLHasher<const ConceptExistentialQuantificationImpl*>
{
    size_t operator()(const ConceptExistentialQuantificationImpl* e) const;
};

template<>
struct UniqueDLHasher<const ConceptRoleValueMapContainmentImpl*>
{
    size_t operator()(const ConceptRoleValueMapContainmentImpl* e) const;
};

template<>
struct UniqueDLHasher<const ConceptRoleValueMapEqualityImpl*>
{
    size_t operator()(const ConceptRoleValueMapEqualityImpl* e) const;
};

template<>
struct UniqueDLHasher<const ConceptNominalImpl*>
{
    size_t operator()(const ConceptNominalImpl* e) const;
};

/* Roles */

template<>
struct UniqueDLHasher<const RoleUniversalImpl*>
{
    size_t operator()(const RoleUniversalImpl* e) const;
};

template<PredicateTag P>
struct UniqueDLHasher<const RoleAtomicStateImpl<P>*>
{
    size_t operator()(const RoleAtomicStateImpl<P>* e) const;
};

template<PredicateTag P>
struct UniqueDLHasher<const RoleAtomicGoalImpl<P>*>
{
    size_t operator()(const RoleAtomicGoalImpl<P>* e) const;
};

template<>
struct UniqueDLHasher<const RoleIntersectionImpl*>
{
    size_t operator()(const RoleIntersectionImpl* e) const;
};

template<>
struct UniqueDLHasher<const RoleUnionImpl*>
{
    size_t operator()(const RoleUnionImpl* e) const;
};

template<>
struct UniqueDLHasher<const RoleComplementImpl*>
{
    size_t operator()(const RoleComplementImpl* e) const;
};

template<>
struct UniqueDLHasher<const RoleInverseImpl*>
{
    size_t operator()(const RoleInverseImpl* e) const;
};

template<>
struct UniqueDLHasher<const RoleCompositionImpl*>
{
    size_t operator()(const RoleCompositionImpl* e) const;
};

template<>
struct UniqueDLHasher<const RoleTransitiveClosureImpl*>
{
    size_t operator()(const RoleTransitiveClosureImpl* e) const;
};

template<>
struct UniqueDLHasher<const RoleReflexiveTransitiveClosureImpl*>
{
    size_t operator()(const RoleReflexiveTransitiveClosureImpl* e) const;
};

template<>
struct UniqueDLHasher<const RoleRestrictionImpl*>
{
    size_t operator()(const RoleRestrictionImpl* e) const;
};

template<>
struct UniqueDLHasher<const RoleIdentityImpl*>
{
    size_t operator()(const RoleIdentityImpl* e) const;
};

/**
 * DL grammar constructors
 */

template<ConstructorTag D>
struct UniqueDLHasher<const grammar::DerivationRuleImpl<D>*>
{
    size_t operator()(const grammar::DerivationRuleImpl<D>* e) const;
};

template<ConstructorTag D>
struct UniqueDLHasher<const grammar::NonTerminalImpl<D>*>
{
    size_t operator()(const grammar::NonTerminalImpl<D>* e) const;
};

template<ConstructorTag D>
struct UniqueDLHasher<const grammar::ConstructorOrNonTerminalChoice<D>*>
{
    size_t operator()(const grammar::ConstructorOrNonTerminalChoice<D>* e) const;
};

template<ConstructorTag D>
struct UniqueDLHasher<const grammar::ChoiceImpl<D>*>
{
    size_t operator()(const grammar::ChoiceImpl<D>* e) const;
};

/* Concepts */

template<>
struct UniqueDLHasher<const grammar::ConceptBotImpl*>
{
    size_t operator()(const grammar::ConceptBotImpl* e) const;
};

template<>
struct UniqueDLHasher<const grammar::ConceptTopImpl*>
{
    size_t operator()(const grammar::ConceptTopImpl* e) const;
};

template<PredicateTag P>
struct UniqueDLHasher<const grammar::ConceptAtomicStateImpl<P>*>
{
    size_t operator()(const grammar::ConceptAtomicStateImpl<P>* e) const;
};

template<PredicateTag P>
struct UniqueDLHasher<const grammar::ConceptAtomicGoalImpl<P>*>
{
    size_t operator()(const grammar::ConceptAtomicGoalImpl<P>* e) const;
};

template<>
struct UniqueDLHasher<const grammar::ConceptIntersectionImpl*>
{
    size_t operator()(const grammar::ConceptIntersectionImpl* e) const;
};

template<>
struct UniqueDLHasher<const grammar::ConceptUnionImpl*>
{
    size_t operator()(const grammar::ConceptUnionImpl* e) const;
};

template<>
struct UniqueDLHasher<const grammar::ConceptNegationImpl*>
{
    size_t operator()(const grammar::ConceptNegationImpl* e) const;
};

template<>
struct UniqueDLHasher<const grammar::ConceptValueRestrictionImpl*>
{
    size_t operator()(const grammar::ConceptValueRestrictionImpl* e) const;
};

template<>
struct UniqueDLHasher<const grammar::ConceptExistentialQuantificationImpl*>
{
    size_t operator()(const grammar::ConceptExistentialQuantificationImpl* e) const;
};

template<>
struct UniqueDLHasher<const grammar::ConceptRoleValueMapContainmentImpl*>
{
    size_t operator()(const grammar::ConceptRoleValueMapContainmentImpl* e) const;
};

template<>
struct UniqueDLHasher<const grammar::ConceptRoleValueMapEqualityImpl*>
{
    size_t operator()(const grammar::ConceptRoleValueMapEqualityImpl* e) const;
};

template<>
struct UniqueDLHasher<const grammar::ConceptNominalImpl*>
{
    size_t operator()(const grammar::ConceptNominalImpl* e) const;
};

/* Roles */

template<>
struct UniqueDLHasher<const grammar::RoleUniversalImpl*>
{
    size_t operator()(const grammar::RoleUniversalImpl* e) const;
};

template<PredicateTag P>
struct UniqueDLHasher<const grammar::RoleAtomicStateImpl<P>*>
{
    size_t operator()(const grammar::RoleAtomicStateImpl<P>* e) const;
};

template<PredicateTag P>
struct UniqueDLHasher<const grammar::RoleAtomicGoalImpl<P>*>
{
    size_t operator()(const grammar::RoleAtomicGoalImpl<P>* e) const;
};

template<>
struct UniqueDLHasher<const grammar::RoleIntersectionImpl*>
{
    size_t operator()(const grammar::RoleIntersectionImpl* e) const;
};

template<>
struct UniqueDLHasher<const grammar::RoleUnionImpl*>
{
    size_t operator()(const grammar::RoleUnionImpl* e) const;
};

template<>
struct UniqueDLHasher<const grammar::RoleComplementImpl*>
{
    size_t operator()(const grammar::RoleComplementImpl* e) const;
};

template<>
struct UniqueDLHasher<const grammar::RoleInverseImpl*>
{
    size_t operator()(const grammar::RoleInverseImpl* e) const;
};

template<>
struct UniqueDLHasher<const grammar::RoleCompositionImpl*>
{
    size_t operator()(const grammar::RoleCompositionImpl* e) const;
};

template<>
struct UniqueDLHasher<const grammar::RoleTransitiveClosureImpl*>
{
    size_t operator()(const grammar::RoleTransitiveClosureImpl* e) const;
};

template<>
struct UniqueDLHasher<const grammar::RoleReflexiveTransitiveClosureImpl*>
{
    size_t operator()(const grammar::RoleReflexiveTransitiveClosureImpl* e) const;
};

template<>
struct UniqueDLHasher<const grammar::RoleRestrictionImpl*>
{
    size_t operator()(const grammar::RoleRestrictionImpl* e) const;
};

template<>
struct UniqueDLHasher<const grammar::RoleIdentityImpl*>
{
    size_t operator()(const grammar::RoleIdentityImpl* e) const;
};

}

#endif
