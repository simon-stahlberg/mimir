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

#ifndef MIMIR_COMMON_DEREF_SHARED_PTR_HPP_
#define MIMIR_COMMON_DEREF_SHARED_PTR_HPP_

#include <loki/details/utils/equal_to.hpp>
#include <loki/details/utils/hash.hpp>
#include <memory>

namespace mimir
{

/// @brief `DerefSharedPtr` wraps T* a non-owning raw pointer.
///
/// Implicit raw pointer conversion is deleted to avoid accidental usage of std overloads, e.g., std::hash<T*>.
/// Comparison operators are deleted to avoid accidental pointer comparisons.
/// @tparam T is the underlying type.
template<typename T>
class DerefSharedPtr
{
public:
    DerefSharedPtr() noexcept : ptr(nullptr) {}
    DerefSharedPtr(std::shared_ptr<T> ptr) noexcept : ptr(std::move(ptr)) {}

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
    bool operator==(const DerefSharedPtr& other) const noexcept { return *ptr == *other.ptr; }
    bool operator!=(const DerefSharedPtr& other) const noexcept { return !(*this == other); }
    bool operator<(const DerefSharedPtr& other) const noexcept { return *ptr < *other.ptr; }
    bool operator<=(const DerefSharedPtr& other) const noexcept { return *ptr <= *other.ptr; }
    bool operator>(const DerefSharedPtr& other) const noexcept { return *ptr > *other.ptr; }
    bool operator>=(const DerefSharedPtr& other) const noexcept { return *ptr >= *other.ptr; }

    /**
     * Conversions
     */

    // Explicitly delete raw pointer conversions.
    operator T*() const = delete;

    // Conversion to boolean
    explicit operator bool() const noexcept { return ptr != nullptr; }

private:
    std::shared_ptr<T> ptr;
};

}

template<typename T>
struct loki::Hash<mimir::DerefSharedPtr<T>>
{
    size_t operator()(const mimir::DerefSharedPtr<T>& ptr) const { return loki::Hash<std::decay_t<T>>()(*ptr); }
};

template<typename T>
struct loki::EqualTo<mimir::DerefSharedPtr<T>>
{
    bool operator()(const mimir::DerefSharedPtr<T>& lhs, const mimir::DerefSharedPtr<T>& rhs) const { return loki::EqualTo<std::decay_t<T>>()(*lhs, *rhs); }
};

#endif