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

#ifndef CISTA_STORAGE_BYTE_BUFFER_SEGMENTED_HPP_
#define CISTA_STORAGE_BYTE_BUFFER_SEGMENTED_HPP_

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <vector>

namespace cista::storage
{

using NumBytes = size_t;

class ByteBufferSegmented
{
private:
    NumBytes m_num_bytes_per_segment;
    NumBytes m_maximum_num_bytes_per_segment;
    std::vector<std::vector<uint8_t>> m_segments;

    size_t m_cur_segment_id;
    size_t m_cur_segment_pos;

    size_t m_size;
    size_t m_capacity;

    size_t m_last_written;

    void increase_capacity(size_t required_bytes)
    {
        if (required_bytes > m_maximum_num_bytes_per_segment)
        {
            throw std::out_of_range("ByteBufferSegmented::increase_capacity: tried to increase capacity beyond maximum bytes per segment: "
                                    + std::to_string(required_bytes) + " > " + std::to_string(m_maximum_num_bytes_per_segment));
        }

        // Ensure that required bytes fit into a buffer.
        m_num_bytes_per_segment = std::max(required_bytes, m_num_bytes_per_segment);

        // Use doubling strategy to make future insertions cheaper.
        m_num_bytes_per_segment = std::min(2 * m_num_bytes_per_segment, m_maximum_num_bytes_per_segment);

        m_segments.push_back(std::vector<uint8_t>(m_num_bytes_per_segment));
        m_capacity += m_num_bytes_per_segment;
        m_cur_segment_pos = 0;
        ++m_cur_segment_id;
    }

public:
    explicit ByteBufferSegmented(NumBytes initial_num_bytes_per_segment = 1024, NumBytes maximum_num_bytes_per_segment = 1024 * 1024) :
        m_num_bytes_per_segment(initial_num_bytes_per_segment),
        m_maximum_num_bytes_per_segment(maximum_num_bytes_per_segment),
        m_segments(),
        m_cur_segment_id(-1),
        m_cur_segment_pos(0),
        m_size(0),
        m_capacity(0),
        m_last_written(0)
    {
    }

    /// @brief Write the data starting from the m_cur_segment_pos
    ///        in the segment with m_cur_segment_id, if it fits,
    ///        and otherwise, push_back a new segment first.
    uint8_t* write(const uint8_t* data, size_t amount)
    {
        assert(data);
        if ((m_segments.size() == 0) || (amount > (m_num_bytes_per_segment - m_cur_segment_pos)))
        {
            increase_capacity(amount);
        }
        uint8_t* result_data = &m_segments[m_cur_segment_id][m_cur_segment_pos];
        memcpy(result_data, data, amount);
        m_cur_segment_pos += amount;
        m_size += amount;
        m_last_written = amount;
        return result_data;
    }

    /// @brief Undo the last write operation.
    void undo_last_write()
    {
        m_cur_segment_pos -= m_last_written;
        m_last_written = 0;
    }

    /// @brief Set the write head to the beginning.
    void clear()
    {
        m_segments.clear();
        m_cur_segment_id = -1;
        m_cur_segment_pos = 0;
        m_size = 0;
        m_capacity = 0;
        m_last_written = 0;
    }

    size_t num_segments() const { return m_segments.size(); }
    size_t size() const { return m_size; }
    size_t capacity() const { return m_capacity; }
};
}

#endif
