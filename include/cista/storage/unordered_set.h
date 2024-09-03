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
/// @brief `DerefStdHasher` is a hash function tailored towards `UnorderedSet`
/// which should always accept const pointer types and computes a hash for the object pointed to.
/// @tparam T
template<typename T>
struct DerefStdHasher
{
    size_t operator()(const T* ptr) const { return std::hash<T>()(*ptr); }
};

/// @brief `DerefStdEqualTo` is a comparison function tailored towards `UnorderedSet`
/// which should always accept const pointer types and compares the objects pointed to.
/// @tparam T
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
    std::unordered_set<const T*, Hash, Equal> m_elements;

    // Serialization buffer
    cista::buf<std::vector<uint8_t>> m_buf;

    using iterator = typename std::unordered_set<const T*, Hash, Equal>::iterator;
    using const_iterator = typename std::unordered_set<const T*, Hash, Equal>::const_iterator;

public:
    explicit UnorderedSet(size_t initial_num_bytes_per_segment = 1024, size_t maximum_num_bytes_per_segment = 1024 * 1024) {}
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

    template<cista::mode Mode = cista::mode::NONE>
    std::pair<const_iterator, bool> insert(const T& element)
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
            throw std::logic_error("cista::storage::UnorderedSet::insert: serialized buffer before write does not satisfy alignment requirements.");
        }

        /* Write the data to the storage and return it. */
        auto begin = m_storage.write(m_buf.base(), m_buf.size());
        if (reinterpret_cast<uintptr_t>(begin) % alignof(T) != 0)
        {
            throw std::logic_error("cista::storage::UnorderedSet::insert: serialized buffer after write does not satisfy alignment requirements.");
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

    const ByteBufferSegmented& get_storage() const { return m_storage; }
};

}

#endif
