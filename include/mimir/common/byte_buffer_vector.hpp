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

#ifndef MIMIR_COMMON_BYTE_BUFFER_VECTOR_HPP_
#define MIMIR_COMMON_BYTE_BUFFER_VECTOR_HPP_

#include "mimir/common/byte_buffer_segmented.hpp"

#include <optional>

namespace mimir
{

template<typename T>
class ByteBufferVector
{
private:
    // Persistent storage
    ByteBufferSegmented m_storage;

    // Data to be accessed
    std::vector<uint8_t*> m_element_begin;

    void range_check(size_t pos) const;

public:
    /// @brief Constructor that ensure that a resize yields non trivially initialized objects.
    ByteBufferVector(NumBytes initial_num_bytes_per_segment = 1024, NumBytes maximum_num_bytes_per_segment = 1024 * 1024) :
        m_storage(initial_num_bytes_per_segment, maximum_num_bytes_per_segment),
        m_element_begin()
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

    std::optional<std::pair<uint8_t*, uint8_t*>> operator[](size_t pos)
    {
        if (!m_element_begin[pos])
        {
            return std::nullopt;
        }
        return { m_element_begin[pos], m_element_begin[pos + 1] };
    }
    std::optional<std::pair<const uint8_t*, const uint8_t*>> operator[](size_t pos) const
    {
        if (!m_element_begin[pos])
        {
            return std::nullopt;
        }
        return { m_element_begin[pos], m_element_begin[pos + 1] };
    }

    std::optional<std::pair<uint8_t*, uint8_t*>> at(size_t pos)
    {
        range_check(pos);
        return (this)[pos];
    }
    std::optional<std::pair<const uint8_t*, const uint8_t*>> at(size_t pos) const
    {
        range_check(pos);
        return (this)[pos];
    }

    const ByteBufferSegmented& get_storage() const { return m_storage; }

    /**
     * Capacity
     */

    constexpr size_t empty() const { return m_element_begin.empty(); }
    constexpr size_t size() const { return m_element_begin.size() - 1; }

    /**
     * Modifiers
     */

    void push_back(const std::vector<uint8_t>& buffer)
    {
        m_element_begin.resize(size() + 2);
        auto begin = m_storage.write(buffer.data(), buffer.size());
        m_element_begin[size()] = begin;
        m_element_begin[size() + 1] = begin + buffer.size();
    }
    void resize(size_t count, const std::vector<uint8_t>& value)
    {
        if (count < size())
        {
            throw std::runtime_error("ByteBufferVector::resize: shrinking not allowed because partial deletion is not supported.");
        }
        while (size() < count)
        {
            push_back(value);
        }
    }
    void clear()
    {
        m_storage.clear();
        m_element_begin.clear();
    }
};

}

#endif
