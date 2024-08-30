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

#ifndef CISTA_STORAGE_VECTOR_HPP_
#define CISTA_STORAGE_VECTOR_HPP_

#include "cista/serialization.h"
#include "cista/storage/byte_buffer_segmented.h"

namespace cista::storage
{

/// @brief `ByteBufferVector` is a container that stores buffers of a cista container of type T.
/// It provides random access to the elements and ensures that the serialize and deserialize functions are compatible.
/// @tparam T
template<typename T>
class ByteBufferVector
{
private:
    // Persistent storage
    ByteBufferSegmented m_storage;

    // Data to be accessed
    std::vector<T*> m_elements;

    // Serialization buffer
    cista::buf<std::vector<uint8_t>> m_buf;

    void range_check(size_t pos) const {}

public:
    /// @brief Constructor that ensure that a resize yields non trivially initialized objects.
    ByteBufferVector(NumBytes initial_num_bytes_per_segment = 1024, NumBytes maximum_num_bytes_per_segment = 1024 * 1024) :
        m_storage(initial_num_bytes_per_segment, maximum_num_bytes_per_segment),
        m_elements(),
        m_buf()
    {
    }
    // Move only to avoid invalidating views.
    ByteBufferVector(const ByteBufferVector& other) = default;
    ByteBufferVector& operator=(const ByteBufferVector& other) = default;
    ByteBufferVector(ByteBufferVector&& other) = default;
    ByteBufferVector& operator=(ByteBufferVector&& other) = default;

    /**
     * Element access
     */

    T* operator[](size_t pos) { return m_elements[pos]; }
    const T* operator[](size_t pos) const { return m_elements[pos]; }

    T* at(size_t pos)
    {
        range_check(pos);
        return (*this)[pos];
    }
    const T* at(size_t pos) const
    {
        range_check(pos);
        return (*this)[pos];
    }

    const ByteBufferSegmented& get_storage() const { return m_storage; }

    /**
     * Capacity
     */

    size_t empty() const { return m_elements.empty(); }
    size_t size() const { return m_elements.size(); }

    /**
     * Modifiers
     */

    void push_back(const T& element)
    {
        /* Serialize the element. */
        m_buf.reset();
        cista::serialize(m_buf, element);

        /* Add padding to ensure that subsequent elements are aligned correctly. */
        size_t num_padding = (alignof(T) - (m_buf.size() % alignof(T))) % alignof(T);
        m_buf.buf_.insert(m_buf.buf_.end(), num_padding, 0);
        assert(m_buf.size() % alignof(T) == 0);

        /* Write the data to the storage. */
        auto begin = m_storage.write(m_buf.base(), m_buf.size());
        assert(reinterpret_cast<uintptr_t>(begin) % alignof(T) == 0);

        /* Add the deserialized element to the vector. */
        m_elements.push_back(cista::deserialize<T>(begin, begin + m_buf.size()));
    }
    void clear()
    {
        m_storage.clear();
        m_elements.clear();
    }
};

}

#endif
