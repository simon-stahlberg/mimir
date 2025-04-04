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

#ifndef MIMIR_BUFFERING_UNORDERED_SET_HPP_
#define MIMIR_BUFFERING_UNORDERED_SET_HPP_

#include "cista/serialization.h"
#include "mimir/buffering/byte_buffer_segmented.h"
#include "mimir/common/hash.hpp"
#include "mimir/common/memory.hpp"

#include <absl/container/flat_hash_set.h>
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
template<typename T, typename Hash = loki::Hash<loki::ObserverPtr<const T>>, typename Equal = loki::EqualTo<loki::ObserverPtr<const T>>>
class UnorderedSet
{
private:
    // Persistent storage
    ByteBufferSegmented m_storage;

    // Data to be accessed, we use absl::flat_hash_set because it stores the data in contiguous memory.
    absl::flat_hash_set<loki::ObserverPtr<const T>, Hash, Equal> m_elements;

    // Serialization buffer
    cista::buf<std::vector<uint8_t>> m_buf;

public:
    explicit UnorderedSet(size_t initial_num_bytes_per_segment = 1024, size_t maximum_num_bytes_per_segment = 1024 * 1024) {}
    UnorderedSet(const UnorderedSet& other) = delete;
    UnorderedSet& operator=(const UnorderedSet& other) = delete;
    UnorderedSet(UnorderedSet&& other) = default;
    UnorderedSet& operator=(UnorderedSet&& other) = default;

    /**
     * Iterators
     */

    auto begin() { return m_elements.begin(); }
    auto begin() const { return m_elements.begin(); }
    auto end() { return m_elements.end(); }
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

    template<cista::mode Mode = cista::mode::NONE>
    auto insert(const T& element)
    {
        /* Check whether element exists already. */
        auto it = m_elements.find(&element);
        if (it != m_elements.end())
        {
            return std::make_pair(it, false);
        }

        /* Serialize the element. */
        m_buf.reset();
        cista::serialize<Mode>(m_buf, element);

        /* Add padding to ensure that subsequent elements are aligned correctly. */
        size_t num_padding = (alignof(T) - (m_buf.size() % alignof(T))) % alignof(T);
        m_buf.buf_.insert(m_buf.buf_.end(), num_padding, 0);
        if (m_buf.size() % alignof(T) != 0)
        {
            throw std::logic_error("mimir::buffering::UnorderedSet::insert: serialized buffer before write does not satisfy alignment requirements.");
        }

        /* Write the data to the storage and return it. */
        auto begin = m_storage.write(m_buf.base(), m_buf.size());
        if (reinterpret_cast<uintptr_t>(begin) % alignof(T) != 0)
        {
            throw std::logic_error("mimir::buffering::UnorderedSet::insert: serialized buffer after write does not satisfy alignment requirements.");
        }

        /* Add the deserialized element to the unordered_set and return it. */
        return m_elements.insert(cista::deserialize<const T, Mode>(begin, begin + m_buf.size()));
    }

    /**
     * Lookup
     */

    size_t count(const T& key) const { return m_elements.count(&key); }
    auto find(const T& key) { return m_elements.find(&key); }
    auto find(const T& key) const { return m_elements.find(&key); }
    bool contains(const T& key) const { return m_elements.contains(&key); }

    size_t get_estimated_memory_usage_in_bytes() const
    {
        const auto usage1 = m_storage.capacity();
        const auto usage2 = mimir::get_memory_usage_in_bytes(m_elements);
        return usage1 + usage2;
    }
};

}

#endif
