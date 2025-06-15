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

#ifndef MIMIR_BUFFERING_UNORDERED_MAP_HPP_
#define MIMIR_BUFFERING_UNORDERED_MAP_HPP_

#include "mimir/buffering/builder.hpp"
#include "mimir/buffering/byte_buffer_segmented.hpp"
#include "mimir/common/hash.hpp"

#include <absl/container/flat_hash_map.h>
#include <functional>
#include <loki/details/utils/equal_to.hpp>
#include <loki/details/utils/hash.hpp>
#include <utility>
#include <vector>

namespace mimir::buffering
{
/// @brief `UnorderedSet` is a container that uniquely stores buffers of a cista container of type T.
/// @tparam T is the underlying container type.
/// @tparam Hash is a hash function that computes a hash value for a dereferenced pointer of type T.
/// @tparam Equal is a comparison function that compares two dereferenced pointers of type T.
template<typename T, typename Value, typename Hash = loki::Hash<View<T>>, typename Equal = loki::EqualTo<View<T>>>
class UnorderedMap
{
private:
    // Persistent storage
    ByteBufferSegmented m_storage;

    // Data to be accessed, we use absl::flat_hash_set because it stores the data in contiguous memory.
    absl::flat_hash_map<View<T>, Value, Hash, Equal> m_elements;

public:
    explicit UnorderedMap(size_t initial_num_bytes_per_segment = 1024, size_t maximum_num_bytes_per_segment = 1024 * 1024) {}
    UnorderedMap(const UnorderedMap& other) = delete;
    UnorderedMap& operator=(const UnorderedMap& other) = delete;
    UnorderedMap(UnorderedMap&& other) = default;
    UnorderedMap& operator=(UnorderedMap&& other) = default;

    /**
     * Iterators
     */

    auto begin() const { return m_elements.begin(); }
    auto end() const { return m_elements.end(); }

    /**
     * Capacity
     */

    bool empty() const { return m_elements.empty(); }
    size_t size() const { return m_elements.size(); }

    /**
     * Modifiers
     */

    void clear()
    {
        m_storage.clear();
        m_elements.clear();
    }

    auto emplace(const Builder<T>& builder, const Value& value)
    {
        /* Check whether element exists already. */
        auto it = m_elements.find(View<T>(builder));
        if (it != m_elements.end())
        {
            return std::make_pair(it, false);
        }

        /* Serialize the element. */
        auto& buffer = builder.get_buffer_writer().get_buffer();

        /* Write the data to the storage and return it. */
        auto address = m_storage.write(buffer.data(), buffer.size());

        /* Add the deserialized element to the unordered_set and return it. */
        return m_elements.emplace(View<T>(address), value);
    }

    /**
     * Lookup
     */

    size_t count(const Builder<T>& builder) const { return m_elements.count(View<T>(builder)); }
    auto find(const Builder<T>& builder) const { return m_elements.find(View<T>(builder)); }
    bool contains(const Builder<T>& builder) const { return m_elements.contains(View<T>(builder)); }
};

}

#endif
