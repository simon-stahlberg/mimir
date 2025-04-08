/*
 * Copyright (C) 2023 Dominik Drexler
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
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef MIMIR_COMMON_DEREF_OBSERVER_PTR_HPP_
#define MIMIR_COMMON_DEREF_OBSERVER_PTR_HPP_

#include <loki/details/utils/equal_to.hpp>
#include <loki/details/utils/hash.hpp>

namespace mimir
{

/// @brief `ObserverPtr` wraps T* a non-owning raw pointer.
///
/// Implicit raw pointer conversion is deleted to avoid accidental usage of std overloads, e.g., std::hash<T*>.
/// Comparison operators are deleted to avoid accidental pointer comparisons.
/// @tparam T is the underlying type.
template<typename T>
class DerefObserverPtr
{
public:
    DerefObserverPtr() noexcept : ptr(nullptr) {}
    DerefObserverPtr(T* ptr) noexcept : ptr(ptr) {}  ///< allow implicit conversion from raw pointer

    /**
     * Accessors
     */

    T* get() noexcept { return ptr; }
    const T* get() const noexcept { return ptr; }

    T& operator*() { return *ptr; }
    T* operator->() { return ptr; }
    const T& operator*() const { return *ptr; }
    const T* operator->() const { return ptr; }

    /**
     * Comparisons
     */

    // Explicitly delete comparison to require user defined comparisons.
    bool operator==(const DerefObserverPtr& other) const noexcept { return *ptr == *other.ptr; }
    bool operator!=(const DerefObserverPtr& other) const noexcept { return !(*this == other); }
    bool operator<(const DerefObserverPtr& other) const noexcept { return *ptr < *other.ptr; }
    bool operator<=(const DerefObserverPtr& other) const noexcept { return *ptr <= *other.ptr; }
    bool operator>(const DerefObserverPtr& other) const noexcept { return *ptr > *other.ptr; }
    bool operator>=(const DerefObserverPtr& other) const noexcept { return *ptr >= *other.ptr; }

    /**
     * Conversions
     */

    // Explicitly delete raw pointer conversions.
    operator T*() const = delete;

    // Conversion to boolean
    explicit operator bool() const noexcept { return ptr != nullptr; }

private:
    T* ptr;
};

}

template<typename T>
struct loki::Hash<mimir::DerefObserverPtr<T>>
{
    size_t operator()(const mimir::DerefObserverPtr<T>& ptr) const { return loki::Hash<std::decay_t<T>>()(*ptr); }
};

template<typename T>
struct loki::EqualTo<mimir::DerefObserverPtr<T>>
{
    bool operator()(const mimir::DerefObserverPtr<T>& lhs, const mimir::DerefObserverPtr<T>& rhs) const { return loki::EqualTo<std::decay_t<T>>()(*lhs, *rhs); }
};

#endif