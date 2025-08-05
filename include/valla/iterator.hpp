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

#ifndef VALLA_INCLUDE_ITERATOR_HPP_
#define VALLA_INCLUDE_ITERATOR_HPP_

#include <concepts>
#include <vector>

namespace valla
{

template<std::unsigned_integral I>
struct Entry
{
    I m_index;
    I m_size;

    Entry(I index, I size) : m_index(index), m_size(size) {}
};

template<typename T>
inline void copy_object(const std::vector<T>& src, std::vector<T>& dst)
{
    dst.clear();
    dst.insert(dst.end(), src.begin(), src.end());
}

}

#endif