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

template<typename T, bool Deref = false, bool Deep = false>
struct Hash;

/**
 * Hashing utilities
 */

template<bool Deref = false, bool Deep = false>
struct HashCombiner
{
public:
    template<typename T>
    void operator()(size_t& seed, const T& value) const
    {
        seed ^= mimir::Hash<T, Deref, Deep>()(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
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
concept IsHashable = requires(T a)
{
    {
        a.hash()
        } -> std::same_as<size_t>;
};

template<typename T>
concept IsShallowHashable = requires(T a)
{
    {
        a.hash_shallow()
        } -> std::same_as<size_t>;
};

template<typename T>
concept IsDeepHashable = requires(T a)
{
    {
        a.hash_deep()
        } -> std::same_as<size_t>;
};

/// @brief `Hash` implements a customizable hashing strategy for types `T`.
///
/// This template struct provides a flexible mechanism for hashing objects of type `T`
/// with optional dereferencing and depth control.
///
/// - If the type `T` provides a member function `hash_deep()` that returns a `size_t`,
///   and if the `Deep` template parameter is set to `true`, this function will be used
///   for hashing. This is intended for scenarios where the entire object, including nested
///   elements, needs to be recursively hashed. When `Deep` is `true`, it is also generally
///   recommended to set `Deref` to `true` to ensure that pointer-like types are dereferenced
///   before hashing, thereby avoiding the default `std::hash` case.
///
/// - If the `Deep` template parameter is set to `false` and the type `T` provides a
///   member function `hash_shallow()` that returns a `size_t`, this function will be used
///   instead. This shallow hashing strategy assumes that nested elements are unique
///   (e.g., by address) and avoids recursion through the entire composite structure.
///
/// - If the type `T` provides a general-purpose `hash()` function that returns a `size_t`,
///   this function will be used for hashing if neither `hash_deep()` nor `hash_shallow()`
///   are available or applicable. This implies that the type `T` is specialized for a specific
///   single use case and does not need to differentiate between deep and shallow hashing contexts.
///
/// - If the `Deref` template parameter is set to `true`, the type `T` must be dereferenceable.
///   In this case, the object of type `T` is dereferenced before computing the hash value.
///   The `Deep` parameter is then applied to the dereferenced object to determine the
///   appropriate hashing strategy.
///
/// - If neither `hash_deep()`, `hash_shallow()`, nor `hash()` are available, the struct falls back
///   to using `std::hash` as a default hashing mechanism for the type `T`.
///
/// - The `Hash` struct provides compile-time checks and constraints, ensuring that only
///   valid combinations of `Deref` and `Deep` are allowed based on the properties of the
///   type `T`.
///
/// @tparam T The type of the object to be hashed.
/// @tparam Deref If `true`, the object is dereferenced before hashing. Applicable only
///         to pointer-like types. For deep hashing (`Deep = true`), it is recommended
///         that `Deref` is also set to `true` if `T` is a pointer type.
/// @tparam Deep If `true`, the `hash_deep()` member function is used for hashing.
///         If `false`, the `hash_shallow()` member function is used instead.
template<typename T, bool Deref, bool Deep>
struct Hash
{
    static_assert(!Deep || Deref);

    size_t operator()(const T& element) const
    {
        if constexpr (Deref && IsDereferencable<T>)
        {
            if (!element)
            {
                throw std::logic_error("Hash<T, Deref>::operator(): Tried to dereference a nullptr.");
            }
            using DereferencedType = std::decay_t<decltype(*element)>;
            return mimir::Hash<DereferencedType, Deref, Deep>()(*element);
        }
        else if constexpr (Deep && IsDeepHashable<T>)
        {
            return element.hash_deep();
        }
        else if constexpr (!Deep && IsShallowHashable<T>)
        {
            return element.hash_shallow();
        }
        else if constexpr (IsHashable<T>)
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
template<typename ForwardRange, bool Deref, bool Deep>
requires std::ranges::forward_range<ForwardRange>
struct Hash<ForwardRange, Deref, Deep>
{
    size_t operator()(const ForwardRange& range) const
    {
        std::size_t aggregated_hash = 0;
        for (const auto& item : range)
        {
            mimir::HashCombiner<Deref, Deep>()(aggregated_hash, item);
        }
        return aggregated_hash;
    }
};

/// Spezialization for std::variant.
template<typename Variant, bool Deref, bool Deep>
requires IsVariant<Variant>
struct Hash<Variant, Deref, Deep>
{
    size_t operator()(const Variant& variant) const
    {
        return std::visit([](const auto& arg) { return Hash<std::decay_t<decltype(arg)>, Deref, Deep>()(arg); }, variant);
    }
};

}

#endif
