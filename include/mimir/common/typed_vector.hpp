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

#ifndef MIMIR_COMMON_TYPED_VECTOR_HPP_
#define MIMIR_COMMON_TYPED_VECTOR_HPP_

#include <array>
#include <cstddef>
#include <type_traits>

namespace mimir
{

template<typename T, typename... Keys>
requires std::default_initializable<T>  // Ensure T is default-initializable
class TypedVector
{
private:
    static_assert(sizeof...(Keys) > 0, "At least one key type must be provided.");

    /* Helper to ensure all Keys are distinct */
    template<typename... Types>
    struct are_distinct;

    template<typename First, typename Second, typename... Rest>
    struct are_distinct<First, Second, Rest...> :
        std::bool_constant<!std::is_same_v<First, Second> && are_distinct<First, Rest...>::value && are_distinct<Second, Rest...>::value>
    {
    };

    template<typename Single>
    struct are_distinct<Single> : std::true_type
    {
    };

    static_assert(are_distinct<Keys...>::value, "All keys (Keys) must be distinct types.");

    /* Compile-time check for valid key */
    template<typename Key>
    static constexpr bool is_valid_key()
    {
        return (std::is_same_v<Key, Keys> || ...);
    }

    /* Helper to find the index of a type in a parameter pack */

    template<typename Key, typename... RemainingKeys>
    struct TypeIndex;

    template<typename Key, typename FirstKey, typename... RemainingKeys>
    struct TypeIndex<Key, FirstKey, RemainingKeys...>
    {
        static constexpr size_t value = std::is_same_v<Key, FirstKey> ? 0 : 1 + TypeIndex<Key, RemainingKeys...>::value;
    };

    template<typename Key>
    struct TypeIndex<Key>  // Base case: if Key is not found
    {
        static constexpr size_t value = static_cast<size_t>(-1);  // Indicates failure to find Key
    };

    template<typename Key, typename... RemainingKeys>
    static constexpr size_t find_index()
    {
        constexpr size_t index = TypeIndex<Key, RemainingKeys...>::value;
        static_assert(index != static_cast<size_t>(-1), "Key type Key not found in Keys...");
        return index;
    }

    /* Data */

    std::array<T, sizeof...(Keys)> m_storage;

public:
    TypedVector() : m_storage() {}

    template<typename K>
    T& get()
    {
        static_assert(is_valid_key<K>(), "Key type K must be one of Keys.");

        constexpr size_t index = find_index<K, Keys...>();

        return m_storage[index];
    }

    template<typename K>
    const T& get() const
    {
        static_assert(is_valid_key<K>(), "Key type K must be one of Keys.");

        constexpr size_t index = find_index<K, Keys...>();

        return m_storage[index];
    }
};

}

#endif
