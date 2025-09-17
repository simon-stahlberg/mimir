/*
 * Copyright (C) 2025 Dominik Drexler
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

#ifndef VALLA_INCLUDE_GROWTH_INFO_HPP_
#define VALLA_INCLUDE_GROWTH_INFO_HPP_

#include "valla/utils.hpp"

#include <absl/container/internal/raw_hash_set.h>
#include <cstddef>

namespace valla
{

class GrowthInfo
{
public:
    GrowthInfo(size_t capacity, size_t size = 0) :
        m_capacity(capacity),
        m_size(size),
        m_log2_capacity(std::countr_zero(m_capacity)),
        m_max_size(m_capacity * MAX_LOAD_FACTOR),
        m_growth_left(m_max_size - m_size)
    {
        assert(is_power_of_two(capacity) && capacity >= absl::container_internal::Group::kWidth
               && "capacity must be a power of two and greater or equal to Group::kWidth for wrap around.");
    }

    void increment_size()
    {
        ++m_size;
        --m_growth_left;
    }

    size_t capacity() const { return m_capacity; }
    uint8_t log2_capacity() const { return m_log2_capacity; }
    size_t mask() const { return m_capacity - 1; }
    size_t size() const { return m_size; }
    size_t max_size() const { return m_max_size; }
    size_t growth_left() const { return m_growth_left; }

private:
    size_t m_capacity;
    size_t m_size;

    uint8_t m_log2_capacity;

    size_t m_max_size;
    size_t m_growth_left;

    static constexpr double MAX_LOAD_FACTOR = static_cast<double>(7) / 8;
};

}

#endif