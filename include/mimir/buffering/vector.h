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

#ifndef MIMIR_BUFFERING_VECTOR_HPP_
#define MIMIR_BUFFERING_VECTOR_HPP_

#include "cista/serialization.h"
#include "mimir/buffering/byte_buffer_segmented.h"

namespace mimir::buffering
{

/// @brief `Vector` is a container that stores buffers of a cista container of type T.
/// It provides random access to the elements.
/// @tparam T
template<typename T>
class Vector
{
private:
    // Persistent storage
    ByteBufferSegmented m_storage;

    // Data to be accessed
    std::vector<T*> m_elements;

    // Serialization buffer
    cista::buf<std::vector<uint8_t>> m_buf;

    void range_check(size_t pos) const
    {
        if (pos >= size())
        {
            throw std::out_of_range("mimir::buffering::Vector::range_check: pos (which is " + std::to_string(pos) + ") >= this->size() (which is "
                                    + std::to_string(size()) + ")");
        }
    }

public:
    Vector(size_t initial_num_bytes_per_segment = 1024, size_t maximum_num_bytes_per_segment = 1024 * 1024) :
        m_storage(initial_num_bytes_per_segment, maximum_num_bytes_per_segment),
        m_elements(),
        m_buf()
    {
    }
    Vector(const Vector& other) = default;
    Vector& operator=(const Vector& other) = default;
    Vector(Vector&& other) = default;
    Vector& operator=(Vector&& other) = default;

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

    /**
     * Capacity
     */

    size_t empty() const { return m_elements.empty(); }
    size_t size() const { return m_elements.size(); }

    /**
     * Modifiers
     */

    template<cista::mode Mode = cista::mode::NONE>
    void push_back(const T& element)
    {
        /* Serialize the element. */
        m_buf.reset();
        cista::serialize<Mode>(m_buf, element);

        /* Add padding to ensure that subsequent elements are aligned correctly. */
        size_t num_padding = (alignof(T) - (m_buf.size() % alignof(T))) % alignof(T);
        m_buf.buf_.insert(m_buf.buf_.end(), num_padding, 0);
        if (m_buf.size() % alignof(T) != 0)
        {
            throw std::logic_error("mimir::buffering::Vector::insert: serialized buffer before write does not satisfy alignment requirements.");
        }

        /* Write the data to the storage. */
        auto begin = m_storage.write(m_buf.base(), m_buf.size());
        if (reinterpret_cast<uintptr_t>(begin) % alignof(T) != 0)
        {
            throw std::logic_error("mimir::buffering::Vector::insert: serialized buffer after write does not satisfy alignment requirements.");
        }

        /* Add the deserialized element to the vector. */
        m_elements.push_back(cista::deserialize<T, Mode>(begin, begin + m_buf.size()));
    }
    void clear()
    {
        m_storage.clear();
        m_elements.clear();
    }

    size_t get_estimated_memory_usage_in_bytes() const { return m_storage.capacity() + m_elements.capacity() * sizeof(T*); }
};

}

#endif
