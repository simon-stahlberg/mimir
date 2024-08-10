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

#ifndef MIMIR_COMMON_HASH_HPP_
#define MIMIR_COMMON_HASH_HPP_

#include "mimir/common/concepts.hpp"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <span>
#include <utility>

namespace mimir
{

/**
 * Forward declarations
 */

template<typename T>
struct Hash;

/**
 * Hashing utilities
 */

template<typename T>
inline void hash_combine(size_t& seed, const T& val)
{
    seed ^= mimir::Hash<T>()(val) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

template<>
inline void hash_combine(size_t& seed, const std::size_t& val)
{
    seed ^= val + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

template<typename... Types>
inline size_t hash_combine(const Types&... args)
{
    size_t seed = 0;
    (mimir::hash_combine(seed, args), ...);
    return seed;
}

template<typename T>
concept HasHashMemberFunction = requires(T a)
{
    {
        a.hash()
        } -> std::same_as<size_t>;
};

/**
 * Hasher
 */

/// @brief `Hash` implements hashing of types T.
/// If a type T provides a function hash() that returns a size_t then use it.
/// Otherwise, fallback to using std::hash instead.
/// @tparam T
template<typename T>
struct Hash
{
    size_t operator()(const T& element) const
    {
        if constexpr (HasHashMemberFunction<T>)
        {
            return element.hash();
        }
        else
        {
            return std::hash<T>()(element);
        }
    }
};

/// Spezialization for std::ranges::forward_range.
template<typename ForwardRange>
requires std::ranges::forward_range<ForwardRange>
struct Hash<ForwardRange>
{
    size_t operator()(const ForwardRange& range) const
    {
        std::size_t aggregated_hash = 0;
        for (const auto& item : range)
        {
            mimir::hash_combine(aggregated_hash, item);
        }
        return aggregated_hash;
    }
};

/// Spezialization for std::variant.
template<typename Variant>
requires IsVariant<Variant>
struct Hash<Variant>
{
    size_t operator()(const Variant& variant) const
    {
        return std::visit([](const auto& arg) { return Hash<std::decay_t<decltype(arg)>>()(arg); }, variant);
    }
};

}

#endif
