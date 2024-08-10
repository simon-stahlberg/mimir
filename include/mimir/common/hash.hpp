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

#include <cstddef>
#include <cstdint>
#include <functional>
#include <span>
#include <utility>

namespace mimir
{

template<typename T>
struct Hash;

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

template<class Container>
inline std::size_t hash_container(const Container& container)
{
    using T = typename Container::value_type;
    const auto hash_function = mimir::Hash<T>();
    std::size_t aggregated_hash = 0;
    for (const auto& item : container)
    {
        const auto item_hash = hash_function(item);
        hash_combine(aggregated_hash, item_hash);
    }
    return aggregated_hash;
}

template<typename T>
inline std::size_t hash_span(const std::span<T>& span)
{
    const auto hash_function = mimir::Hash<T>();
    std::size_t aggregated_hash = 0;
    for (const auto& item : span)
    {
        const auto item_hash = hash_function(item);
        hash_combine(aggregated_hash, item_hash);
    }
    return aggregated_hash;
}

template<typename Container>
struct hash_container_type
{
    size_t operator()(const Container& container) const { return mimir::hash_container(container); }
};

template<typename T>
concept HasHashMemberFunction = requires(T a)
{
    {
        a.hash()
        } -> std::same_as<size_t>;
};

/// @brief `Hash` implements hashing of types T.
/// If a type satisfies the `HasHashMemberFunction` then it will call it.
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

template<typename T>
struct Hash<std::span<T>>
{
    size_t operator()(const std::span<T>& span) const { return hash_span(span); }
};

}

#endif
