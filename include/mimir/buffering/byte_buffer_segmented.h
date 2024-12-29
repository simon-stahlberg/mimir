/*
 * Copyright (C) 2024 Dominik Drexler
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

#ifndef MIMIR_BUFFERING_BYTE_BUFFER_SEGMENTED_HPP_
#define MIMIR_BUFFERING_BYTE_BUFFER_SEGMENTED_HPP_

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <vector>

namespace mimir::buffering
{

class ByteBufferSegmented
{
private:
    size_t m_num_bytes_per_segment;
    size_t m_maximum_num_bytes_per_segment;

    std::vector<std::vector<uint8_t>> m_segments;

    size_t m_current_segment_index;
    size_t m_current_segment_position;

    size_t m_size;
    size_t m_capacity;

    void increase_capacity(size_t required_bytes)
    {
        if (required_bytes > m_maximum_num_bytes_per_segment)
        {
            throw std::out_of_range("mimir::buffering::ByteBufferSegmented::increase_capacity: tried to increase capacity beyond maximum bytes per segment: "
                                    + std::to_string(required_bytes) + " > " + std::to_string(m_maximum_num_bytes_per_segment));
        }

        // Ensure that required bytes fit into a buffer.
        m_num_bytes_per_segment = std::max(required_bytes, m_num_bytes_per_segment);

        // Use doubling strategy to make future insertions cheaper.
        m_num_bytes_per_segment = std::min(2 * m_num_bytes_per_segment, m_maximum_num_bytes_per_segment);

        m_segments.push_back(std::vector<uint8_t>(m_num_bytes_per_segment));

        m_capacity += m_num_bytes_per_segment;
        m_current_segment_position = 0;
        ++m_current_segment_index;
    }

public:
    explicit ByteBufferSegmented(size_t initial_num_bytes_per_segment = 1024, size_t maximum_num_bytes_per_segment = 32 * 1024 * 1024) :
        m_num_bytes_per_segment(initial_num_bytes_per_segment),
        m_maximum_num_bytes_per_segment(maximum_num_bytes_per_segment),
        m_segments(),
        m_current_segment_index(-1),
        m_current_segment_position(0),
        m_size(0),
        m_capacity(0)
    {
        // Reserve the initial segment.
        increase_capacity(initial_num_bytes_per_segment);
    }

    /// @brief Write the data starting from the m_current_segment_position
    ///        in the segment with m_current_segment_index, if it fits,
    ///        and otherwise, push_back a new segment first.
    uint8_t* write(const uint8_t* data, size_t amount)
    {
        if (data == nullptr)
        {
            throw std::invalid_argument("mimir::buffering::ByteBufferSegmented::write(): data cannot be null");
        }

        if (amount > (m_num_bytes_per_segment - m_current_segment_position))
        {
            increase_capacity(amount);
        }

        uint8_t* result_data = &m_segments[m_current_segment_index][m_current_segment_position];

        memcpy(result_data, data, amount);

        m_current_segment_position += amount;
        m_size += amount;

        return result_data;
    }

    /// @brief Set the write head to the beginning.
    void clear()
    {
        m_segments.clear();
        m_current_segment_index = -1;
        m_current_segment_position = 0;
        m_size = 0;
        m_capacity = 0;
    }

    size_t num_segments() const { return m_segments.size(); }
    size_t size() const { return m_size; }
    size_t capacity() const { return m_capacity; }
};
}

#endif
