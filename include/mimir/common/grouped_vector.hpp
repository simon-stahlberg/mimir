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
 * Utility to group elements in a vector and access elements of a group by index.
 *
 * Example:
 *
 * Input:
 *  - vec                     = [(2,3),(0,2),(2,0),(0,2)]
 *  - num_groups              = 4
 *  - sort comparator  (l,r) := if l[0] == r[0]: return l[1] < r[1] else l[0] < r[0]
 *  - group comparator (l,r) := l[0] < r[0]
 *  - group retriever  (x)   := x[0]
 *
 * Output:
 *   - vec       = [(0,2),(0,2),(2,0),(2,3)]
 *   - (indices) = [0,1,2,3]
 *   - groups    = [0,2,2,4,4]
 *
 * Accessors:
 *   - get_groups(0) = span(0,2)
 *   - get_groups(1) = span(2,2)
 *   - get_groups(2) = span(2,4)
 *   - get_groups(3) = span(4,4)
 *   - get_groups(4) = span(4,4)
 */

template<typename T, typename U>
concept IsSortComparator = requires(T a, U u) {
    // Require a comparison operator to obtain a sorted sequence
    {
        a(u, u)
    } -> std::same_as<bool>;
};

template<typename T, typename U>
concept IsGroupComparator = requires(T a, U u) {
    // Require an operator to identify when a new group begins.
    {
        a(u, u)
    } -> std::same_as<bool>;
};

template<typename T, typename U>
concept IsGroupRetriever = requires(T a, U u) {
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

    IndexGroupedVector(std::vector<T> vec, std::vector<size_t> groups_begin) : m_vec(std::move(vec)), m_groups_begin(std::move(groups_begin)) {}

public:
    IndexGroupedVector() = default;

    template<typename SortComparator, typename GroupComparator, typename GroupRetriever>
        requires IsSortComparator<SortComparator, T> && IsGroupComparator<GroupComparator, T> && IsGroupRetriever<GroupRetriever, T>
    static IndexGroupedVector<T>
    create(std::vector<T> vec, size_t num_groups, SortComparator sort_comparator, GroupComparator group_comparator, GroupRetriever group_retriever)
    {
        /* Sort */

        std::sort(vec.begin(), vec.end(), sort_comparator);

        /* Group */

        auto groups_begin = std::vector<size_t> {};
        // Push begin of first group.
        groups_begin.push_back(0);
        for (size_t i = 1; i < vec.size(); ++i)
        {
            const auto cur_group = groups_begin.size() - 1;
            if (cur_group > group_retriever(vec[i]))
            {
                std::logic_error("IndexGroupedVector::create: got element of finished group. Incompatible sort and group comparator.");
            }

            if (group_comparator(vec[i - 1], vec[i]))
            {
                // Write begin i for skipped groups + begin of new group.
                while (groups_begin.size() <= group_retriever(vec[i]))
                {
                    groups_begin.push_back(i);
                }
            }
        }
        // Set begin of remaining groups + end of last group.
        while (groups_begin.size() <= num_groups)
        {
            groups_begin.push_back(vec.size());
        }

        return IndexGroupedVector<T>(std::move(vec), std::move(groups_begin));
    }

    std::span<T> get_group(size_t pos) const
    {
        return std::span<T>(  //
            m_vec.begin() + m_groups_begin.at(pos),
            m_vec.begin() + m_groups_begin.at(pos + 1));
    }

    const std::vector<T> get_vector() const { return m_vec; }
    const std::vector<size_t> get_groups_begin() const { return m_groups_begin; }
};

}

#endif
