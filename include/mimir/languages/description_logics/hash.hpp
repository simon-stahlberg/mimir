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

namespace mimir
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
            UniquePDDLHashCombiner()(aggregated_hash, item);
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

template<IsConceptOrRole D>
struct UniqueDLHasher<const Constructor<D>*>
{
    size_t operator()(const Constructor<D>* e) const;
};

/* Concepts */

template<PredicateCategory P>
struct UniqueDLHasher<const ConceptPredicateState<P>&>
{
    size_t operator()(const ConceptPredicateState<P>& e) const;
};

template<PredicateCategory P>
struct UniqueDLHasher<const ConceptPredicateGoal<P>&>
{
    size_t operator()(const ConceptPredicateGoal<P>& e) const;
};

template<>
struct UniqueDLHasher<const ConceptPredicateAnd&>
{
    size_t operator()(const ConceptPredicateAnd& e) const;
};

/* Roles */

template<PredicateCategory P>
struct UniqueDLHasher<const RolePredicateState<P>&>
{
    size_t operator()(const RolePredicateState<P>& e) const;
};

template<PredicateCategory P>
struct UniqueDLHasher<const RolePredicateGoal<P>&>
{
    size_t operator()(const RolePredicateGoal<P>& e) const;
};

template<>
struct UniqueDLHasher<const RolePredicateAnd&>
{
    size_t operator()(const RolePredicateAnd& e) const;
};

/**
 * DL grammar constructors
 */

namespace grammar
{

template<IsConceptOrRole D>
struct UniqueDLHasher<const Constructor<D>*>
{
    size_t operator()(const Constructor<D>* e) const;
};

template<IsConceptOrRole D>
struct UniqueDLHasher<const DerivationRule<D>*>
{
    size_t operator()(const DerivationRule<D>* e) const;
};

template<IsConceptOrRole D>
struct UniqueDLHasher<const NonTerminal<D>*>
{
    size_t operator()(const NonTerminal<D>* e) const;
};

template<IsConceptOrRole D>
struct UniqueDLHasher<const ConstructorOrNonTerminalChoice<D>*>
{
    size_t operator()(const ConstructorOrNonTerminalChoice<D>* e) const;
};

template<IsConceptOrRole D>
struct UniqueDLHasher<const Choice<D>*>
{
    size_t operator()(const Choice<D>* e) const;
};

/* Concepts */

template<PredicateCategory P>
struct UniqueDLHasher<const ConceptPredicateState<P>&>
{
    size_t operator()(const ConceptPredicateState<P>& e) const;
};

template<PredicateCategory P>
struct UniqueDLHasher<const ConceptPredicateGoal<P>&>
{
    size_t operator()(const ConceptPredicateGoal<P>& e) const;
};

template<>
struct UniqueDLHasher<const ConceptPredicateAnd&>
{
    size_t operator()(const ConceptPredicateAnd& e) const;
};

/* Roles */

template<PredicateCategory P>
struct UniqueDLHasher<const RolePredicateState<P>&>
{
    size_t operator()(const RolePredicateState<P>& e) const;
};

template<PredicateCategory P>
struct UniqueDLHasher<const RolePredicateGoal<P>&>
{
    size_t operator()(const RolePredicateGoal<P>& e) const;
};

template<>
struct UniqueDLHasher<const RolePredicateAnd&>
{
    size_t operator()(const RolePredicateAnd& e) const;
};

}

}

#endif
