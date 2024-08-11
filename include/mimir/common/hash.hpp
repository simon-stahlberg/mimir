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

template<typename T, bool Deref = false>
struct Hash;

/**
 * Hashing utilities
 */

template<bool Deref = false>
struct HashCombiner
{
public:
    template<typename T>
    void operator()(size_t& seed, const T& value) const
    {
        seed ^= mimir::Hash<T, Deref>()(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
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

/**
 * Hasher
 */

template<typename T>
concept HasHashMemberFunction = requires(T a)
{
    {
        a.hash()
        } -> std::same_as<size_t>;
};

/// @brief `Hash` implements hashing of types T.
/// If a type T provides a function hash() that returns a size_t then use it.
/// Otherwise, fallback to using std::hash instead.
/// @tparam T
/// @tparam Deref
template<typename T, bool Deref>
struct Hash
{
    size_t operator()(const T& element) const
    {
        if constexpr (Deref && IsDereferencable<T>)
        {
            if (!element)
            {
                throw std::logic_error("Hash<T, Deref>::operator(): Tried to illegally dereference an object.");
            }
            using DereferencedType = std::decay_t<decltype(*element)>;
            return mimir::Hash<DereferencedType, Deref>()(*element);
        }
        else if constexpr (HasHashMemberFunction<T>)
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
template<typename ForwardRange, bool Deref>
requires std::ranges::forward_range<ForwardRange>
struct Hash<ForwardRange, Deref>
{
    size_t operator()(const ForwardRange& range) const
    {
        std::size_t aggregated_hash = 0;
        for (const auto& item : range)
        {
            mimir::HashCombiner<Deref>()(aggregated_hash, item);
        }
        return aggregated_hash;
    }
};

/// Spezialization for std::variant.
template<typename Variant, bool Deref>
requires IsVariant<Variant>
struct Hash<Variant, Deref>
{
    size_t operator()(const Variant& variant) const
    {
        return std::visit([](const auto& arg) { return Hash<std::decay_t<decltype(arg)>, Deref>()(arg); }, variant);
    }
};

}

#endif
