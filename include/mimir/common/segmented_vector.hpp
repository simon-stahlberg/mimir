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

#ifndef MIMIR_COMMON_SEGMENTED_VECTOR_HPP_
#define MIMIR_COMMON_SEGMENTED_VECTOR_HPP_

#include <bit>
#include <vector>

namespace mimir
{
template<typename T>
class SegmentedVector
{
private:
    std::vector<std::vector<T>> m_segments;
    size_t m_offset;
    size_t m_capacity;
    size_t m_size;

    static constexpr size_t INITIAL_SEGMENT_SIZE = 1024;

    // TODO assert that initial segment size is a power of 2
    static_assert((INITIAL_SEGMENT_SIZE & (INITIAL_SEGMENT_SIZE - 1)) == 0, "INITIAL_SEGMENT_SIZE must be a power of 2");

    static size_t get_index(size_t pos)
    {
        constexpr size_t k = std::countr_zero(INITIAL_SEGMENT_SIZE);
        return std::countr_zero(std::bit_floor(pos + INITIAL_SEGMENT_SIZE)) - k;
    }

    static size_t get_offset(size_t pos)
    {
        constexpr size_t k = std::countr_zero(INITIAL_SEGMENT_SIZE);
        return pos - (((std::bit_floor(pos + INITIAL_SEGMENT_SIZE) >> k) - 1) << k);
    }

    void resize_to_fit()
    {
        const auto remaining_entries = m_segments.back().capacity() - m_offset;

        if (remaining_entries == 0)
        {
            const auto new_segment_size = m_segments.back().size() * 2;

            m_segments.emplace_back();
            m_segments.back().reserve(new_segment_size);
            m_offset = 0;
            m_capacity += new_segment_size;
        }
    }

public:
    SegmentedVector() : m_segments(), m_offset(0), m_capacity(0), m_size(0)
    {
        m_segments.emplace_back();
        m_segments.back().reserve(INITIAL_SEGMENT_SIZE);
    }

    void push_back(const T& element)
    {
        resize_to_fit();

        m_segments.back().push_back(element);

        ++m_offset;
        ++m_size;
    }
    void push_back(T&& element)
    {
        resize_to_fit();

        m_segments.back().push_back(std::move(element));

        ++m_offset;
        ++m_size;
    }

    void pop_back()
    {
        m_segments.back().pop_back();

        --m_offset;
        --m_size;
    }

    const T& operator[](size_t pos) const
    {
        const auto index = get_index(pos);
        const auto offset = get_offset(pos);

        assert(index < m_segments.size() && offset < m_segments[index].size());

        return m_segments[index][offset];
    }
    T& operator[](size_t pos)
    {
        const auto index = get_index(pos);
        const auto offset = get_offset(pos);

        assert(index < m_segments.size() && offset < m_segments[index].size());

        return m_segments[index][offset];
    }

    const T& at(size_t pos) const
    {
        const auto index = get_index(pos);
        const auto offset = get_offset(pos);

        return m_segments.at(index).at(offset);
    }
    T& at(size_t pos)
    {
        const auto index = get_index(pos);
        const auto offset = get_offset(pos);

        return m_segments.at(index).at(offset);
    }

    const T& back() const { return m_segments.back().back(); }
    T& back() { return m_segments.back().back(); }

    size_t capacity() const { return m_capacity; }
    size_t size() const { return m_size; }
};
}

#endif
