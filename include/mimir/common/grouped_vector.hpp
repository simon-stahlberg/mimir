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

#ifndef MIMIR_COMMON_INDEX_GROUPED_VECTOR_HPP_
#define MIMIR_COMMON_INDEX_GROUPED_VECTOR_HPP_

#include "mimir/common/printers.hpp"

#include <algorithm>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <functional>
#include <iostream>
#include <span>
#include <string>
#include <vector>

namespace mimir
{

/**
 * Examples
 */

/* IndexGroupedVector */

// Utility to group elements in a vector and access elements of a group by index.
//
// Input:
//  - vec                     = [(0,2),(0,2),(2,0),(2,3)]
//  - num_groups              = 4
//  - group comparator (l,r) := l[0] != r[0]
//  - group retriever  (x)   := x[0]
// Output:
//   - (indices) = [0,1,2,3]
//   - vec       = [(0,2),(0,2),(2,0),(2,3)]
//   - groups    = [0,2,2,4,4]
// Accessors:
//   - get_groups(0) = span(0,2)
//   - get_groups(1) = span(2,2)
//   - get_groups(2) = span(2,4)
//   - get_groups(3) = span(4,4)
//   - get_groups(4) = span(4,4)

/**
 * Declarations
 */

template<typename T, typename U>
concept IsGroupBoundaryChecker = requires(T a, U u) {
    // Require an operator to identify when a new group begins.
    {
        a(u, u)
    } -> std::same_as<bool>;
};

template<typename T, typename U>
concept IsGroupIndexRetriever = requires(T a, U u) {
    // Require an operator to retrieve the index of T to be able to add skipped group begin indices.
    {
        a(u)
    } -> std::unsigned_integral;
};

template<typename T>
class IndexGroupedVector
{
private:
    std::vector<T> m_vec;
    std::vector<size_t> m_groups_begin;

    IndexGroupedVector(std::vector<T> vec, std::vector<size_t> groups_begin);

    /// @brief Throws an exception if adding a begin index would be invalid.
    static void range_check(size_t cur_num_groups, size_t num_groups);

public:
    IndexGroupedVector();

    /// @brief This implementation creates a new group when the boundary checker returns true,
    /// uses the group retriever to find the index of the group, and adds empty suffix groups.
    /// @tparam GroupBoundaryChecker
    /// @tparam GroupIndexRetriever
    /// @param vec
    /// @param num_groups
    /// @param group_boundary_checker
    /// @param group_index_retriever
    /// @return
    template<typename GroupBoundaryChecker, typename GroupIndexRetriever>
        requires IsGroupBoundaryChecker<GroupBoundaryChecker, T> && IsGroupIndexRetriever<GroupIndexRetriever, T>
    static IndexGroupedVector<T>
    create(std::vector<T> vec, size_t num_groups, GroupBoundaryChecker group_boundary_checker, GroupIndexRetriever group_index_retriever);

    /// @brief This implementation adds a new group whenever the boundary checker returns true.
    /// @tparam GroupBoundaryChecker
    /// @param vec
    /// @param group_boundary_checker
    /// @return
    template<typename GroupBoundaryChecker>
        requires IsGroupBoundaryChecker<GroupBoundaryChecker, T>
    static IndexGroupedVector<T> create(std::vector<T> vec, GroupBoundaryChecker group_boundary_checker);

    std::span<const T> get_group(size_t pos) const;
    const std::vector<T>& get_vector() const;
    const std::vector<size_t>& get_groups_begin() const;
    size_t get_num_groups() const;
};

template<typename T>
class IndexGroupedVectorBuilder
{
private:
    std::vector<T> m_vec;
    std::vector<size_t> m_groups_begin;

public:
    IndexGroupedVectorBuilder();

    void add_group_element(T element);

    void start_group();

    void clear();

    IndexGroupedVector<T> get_result();
};

/**
 * Implementations
 */

/* IndexGroupedVector */

template<typename T>
IndexGroupedVector<T>::IndexGroupedVector(std::vector<T> vec, std::vector<size_t> groups_begin) : m_vec(std::move(vec)), m_groups_begin(std::move(groups_begin))
{
}

template<typename T>
void IndexGroupedVector<T>::range_check(size_t cur_num_groups, size_t num_groups)
{
    if (cur_num_groups > num_groups)
    {
        throw std::logic_error("IndexGroupedVector::create: Ran out of bounds during grouping.");
    }
}

template<typename T>
IndexGroupedVector<T>::IndexGroupedVector() : m_vec(), m_groups_begin({ 0 })
{
}

template<typename T>
template<typename GroupBoundaryChecker, typename GroupIndexRetriever>
    requires IsGroupBoundaryChecker<GroupBoundaryChecker, T> && IsGroupIndexRetriever<GroupIndexRetriever, T>
inline IndexGroupedVector<T>
IndexGroupedVector<T>::create(std::vector<T> vec, size_t num_groups, GroupBoundaryChecker group_boundary_checker, GroupIndexRetriever group_index_retriever)
{
    auto groups_begin = std::vector<size_t> {};

    if (vec.size() > 0)
    {
        // Write begin of first group.
        range_check(groups_begin.size(), num_groups);
        groups_begin.push_back(0);

        for (size_t i = 1; i < vec.size(); ++i)
        {
            const auto cur_group = groups_begin.size() - 1;
            if (cur_group > group_index_retriever(vec[i]))
            {
                throw std::logic_error("IndexGroupedVector::create: Got element of finished group. Did you forget to sort the input vector?");
            }

            if (group_boundary_checker(vec[i - 1], vec[i]))
            {
                // Write begin i for skipped groups + begin of new group.
                while (groups_begin.size() <= group_index_retriever(vec[i]))
                {
                    range_check(groups_begin.size(), num_groups);
                    groups_begin.push_back(i);
                }
            }
        }
    }

    // Write begin of remaining groups + end of last group.
    range_check(groups_begin.size(), num_groups);
    while (groups_begin.size() <= num_groups)
    {
        groups_begin.push_back(vec.size());
    }

    return IndexGroupedVector<T>(std::move(vec), std::move(groups_begin));
}

template<typename T>
template<typename GroupBoundaryChecker>
    requires IsGroupBoundaryChecker<GroupBoundaryChecker, T>
inline IndexGroupedVector<T> IndexGroupedVector<T>::create(std::vector<T> vec, GroupBoundaryChecker group_boundary_checker)
{
    auto groups_begin = std::vector<size_t> {};

    if (vec.size() > 0)
    {
        // Write begin of first group.
        groups_begin.push_back(0);

        for (size_t i = 1; i < vec.size(); ++i)
        {
            if (group_boundary_checker(vec[i - 1], vec[i]))
            {
                // Write begin of new group.
                groups_begin.push_back(i);
            }
        }
    }

    // Write end of last group.
    groups_begin.push_back(vec.size());

    return IndexGroupedVector<T>(std::move(vec), std::move(groups_begin));
}

template<typename T>
std::span<const T> IndexGroupedVector<T>::get_group(size_t pos) const
{
    return std::span<const T>(  //
        m_vec.begin() + m_groups_begin.at(pos),
        m_vec.begin() + m_groups_begin.at(pos + 1));
}

template<typename T>
const std::vector<T>& IndexGroupedVector<T>::get_vector() const
{
    return m_vec;
}

template<typename T>
const std::vector<size_t>& IndexGroupedVector<T>::get_groups_begin() const
{
    return m_groups_begin;
}

template<typename T>
size_t IndexGroupedVector<T>::get_num_groups() const
{
    return m_groups_begin.size() - 1;
}

/* IndexGroupedVectorBuilder */

template<typename T>
IndexGroupedVectorBuilder<T>::IndexGroupedVectorBuilder() : m_vec(), m_groups_begin()
{
}

template<typename T>
void IndexGroupedVectorBuilder<T>::add_group_element(T element)
{
    m_vec.push_back(std::move(element));
}

template<typename T>
void IndexGroupedVectorBuilder<T>::start_group()
{
    m_groups_begin.push_back(m_vec.size());
}

template<typename T>
void IndexGroupedVectorBuilder<T>::clear()
{
    m_vec.clear();
    m_groups_begin.clear();
}

template<typename T>
IndexGroupedVector<T> IndexGroupedVectorBuilder<T>::get_result()
{
    // Push end of last group.
    m_groups_begin.push_back(m_vec.size());

    return IndexGroupedVector<T>(m_vec, m_groups_begin);
}

}

#endif
