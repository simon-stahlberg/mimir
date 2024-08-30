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

#ifndef CISTA_STORAGE_UNORDERED_SET_HPP_
#define CISTA_STORAGE_UNORDERED_SET_HPP_

#include "cista/serialization.h"
#include "cista/storage/byte_buffer_segmented.h"

#include <unordered_set>
#include <utility>

namespace cista::storage
{
template<typename T>
struct DerefStdHasher
{
    size_t operator()(const T* ptr) const { return std::hash<T>()(*ptr); }
};

template<typename T>
struct DerefStdEqualTo
{
    size_t operator()(const T* lhs, const T* rhs) const { return std::equal_to<T>()(*lhs, *rhs); }
};

/// @brief `UnorderedSet` is a container that uniquely stores buffers of a cista container of type T.
/// @tparam T is the underlying container type.
/// @tparam Hash is a hash function that computes a hash value for a dereferenced pointer of type T.
/// @tparam Equal is a comparison function that compares two dereferenced pointers of type T.
template<typename T, typename Hash = DerefStdHasher<T>, typename Equal = DerefStdEqualTo<T>>
class UnorderedSet
{
private:
    // Persistent storage
    ByteBufferSegmented m_storage;

    // Data to be accessed
    std::unordered_set<T*, Hash, Equal> m_elements;

    // Serialization buffer
    cista::buf<std::vector<uint8_t>> m_buf;

    using iterator = typename std::unordered_set<T*, Hash, Equal>::iterator;
    using const_iterator = typename std::unordered_set<T*, Hash, Equal>::const_iterator;

public:
    explicit UnorderedSet(NumBytes initial_num_bytes_per_segment = 1024, NumBytes maximum_num_bytes_per_segment = 1024 * 1024) {}
    // Move only to avoid invalidating views.
    UnorderedSet(const UnorderedSet& other) = delete;
    UnorderedSet& operator=(const UnorderedSet& other) = delete;
    UnorderedSet(UnorderedSet&& other) = default;
    UnorderedSet& operator=(UnorderedSet&& other) = default;

    /**
     * Iterators
     */

    iterator begin() { return m_elements.begin(); }
    const_iterator begin() const { return m_elements.begin(); }
    iterator end() { return m_elements.end(); }
    const_iterator end() const { return m_elements.end(); }

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

    std::pair<const_iterator, bool> insert(const T& element)
    {
        m_buf.reset();
        cista::serialize(m_buf, element);
        auto begin = m_storage.write(m_buf.base(), m_buf.size());
        return m_elements.insert(cista::deserialize<T>(begin, begin + m_buf.size()));
    }

    /**
     * Lookup
     */

    size_t count(const T& key) const { return m_element.count(key); }
    iterator find(const T& key) { return m_elements.find(key); }
    const_iterator find(const T& key) const { return m_elements.find(&key); }
    bool contains(const T& key) const { return m_elements.contains(&key); }

    const ByteBufferSegmented& get_storage() const { return m_storage; }
};

}

#endif
