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

template<typename T>
class IndexGroupedVectorBuilder;

template<typename T, typename U>
concept IsGroupBoundaryChecker = requires(T a, U u)
{
    // Require an operator to identify when a new group begins.
    {
        a(u, u)
        } -> std::same_as<bool>;
};

template<typename T, typename U>
concept IsGroupIndexRetriever = requires(T a, U u)
{
    // Require an operator to retrieve the index of T to be able to add skipped group begin indices.
    {
        a(u)
        } -> std::unsigned_integral;
};

template<typename T>
class IndexGroupedVector
{
private:
    std::vector<std::remove_const_t<T>> m_vec;
    std::vector<size_t> m_groups_begin;

    IndexGroupedVector(std::vector<std::remove_const_t<T>> vec, std::vector<size_t> groups_begin);

    friend class IndexGroupedVectorBuilder<T>;

    void range_check(size_t pos) const;

public:
    using value_type = std::span<T>;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;

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
    create(std::vector<std::remove_const_t<T>> vec, GroupBoundaryChecker group_boundary_checker, GroupIndexRetriever group_index_retriever, size_t num_groups);

    /// @brief This implementation adds a new group whenever the boundary checker returns true.
    /// @tparam GroupBoundaryChecker
    /// @param vec
    /// @param group_boundary_checker
    /// @return
    template<typename GroupBoundaryChecker>
    requires IsGroupBoundaryChecker<GroupBoundaryChecker, T>
    static IndexGroupedVector<T> create(std::vector<std::remove_const_t<T>> vec, GroupBoundaryChecker group_boundary_checker);

    /**
     * Iterators
     */

    class const_iterator
    {
    private:
        size_t m_pos;
        const IndexGroupedVector<T>* m_parent;

    public:
        using difference_type = std::ptrdiff_t;
        using value_type = typename std::span<const T>;
        using pointer = value_type*;
        using reference = value_type&;
        using iterator_category = std::forward_iterator_tag;

        const_iterator();
        const_iterator(const IndexGroupedVector<T>& parent, bool begin);

        value_type operator*() const;
        const_iterator& operator++();
        const_iterator operator++(int);
        bool operator==(const const_iterator& other) const;
        bool operator!=(const const_iterator& other) const;
    };

    const_iterator begin() const;
    const_iterator end() const;

    /**
     * Accessors
     */

    /// @brief Get group with index pos.
    /// Throw an exception if out of bounds.
    /// @param pos
    /// @return
    std::span<T> at(size_t pos);
    const std::span<const T> at(size_t pos) const;

    /// @brief Get group with index pos.
    /// @param pos
    /// @return
    std::span<T> operator[](size_t pos);
    const std::span<const T> operator[](size_t pos) const;

    /// @brief Get the first group.
    std::span<T> front();
    const std::span<const T> front() const;

    /// @brief Get the last group.
    std::span<T> back();
    const std::span<const T> back() const;

    /// @brief Get the underlying data.
    /// @return
    std::vector<std::remove_const_t<T>>& data();
    const std::vector<std::remove_const_t<T>>& data() const;

    /**
     * Capacity
     */

    /// @brief Get the number of groups.
    /// @return
    size_t size() const;
};

template<typename T>
class IndexGroupedVectorBuilder
{
private:
    std::vector<std::remove_const_t<T>> m_vec;
    std::vector<size_t> m_groups_begin;

public:
    IndexGroupedVectorBuilder();

    /**
     * Modifiers
     */

    void add_group_element(T element);

    size_t start_group();

    void clear();

    /**
     * Access data.
     */

    IndexGroupedVector<T> get_result();

    const std::vector<std::remove_const_t<T>>& data() const;
};

/**
 * Implementations
 */

static inline void create_range_check(size_t cur_num_groups, size_t num_groups)
{
    if (cur_num_groups > num_groups)
    {
        throw std::logic_error("IndexGroupedVector::create: Ran out of bounds during grouping.");
    }
}

/* IndexGroupedVector */

template<typename T>
IndexGroupedVector<T>::IndexGroupedVector(std::vector<std::remove_const_t<T>> vec, std::vector<size_t> groups_begin) :
    m_vec(std::move(vec)),
    m_groups_begin(std::move(groups_begin))
{
}

template<typename T>
void IndexGroupedVector<T>::range_check(size_t pos) const
{
    if (pos >= size())
    {
        throw std::out_of_range("IndexGroupedVector::range_check: pos (which is " + std::to_string(pos) + ") >= this->size() (which is "
                                + std::to_string(size()) + ")");
    }
}

template<typename T>
IndexGroupedVector<T>::IndexGroupedVector() : m_vec(), m_groups_begin({ 0 })
{
}

template<typename T>
template<typename GroupBoundaryChecker, typename GroupIndexRetriever>
requires IsGroupBoundaryChecker<GroupBoundaryChecker, T> && IsGroupIndexRetriever<GroupIndexRetriever, T>
inline IndexGroupedVector<T> IndexGroupedVector<T>::create(std::vector<std::remove_const_t<T>> vec,
                                                           GroupBoundaryChecker group_boundary_checker,
                                                           GroupIndexRetriever group_index_retriever,
                                                           size_t num_groups)
{
    auto groups_begin = std::vector<size_t> {};

    if (vec.size() > 0)
    {
        // Write begin i for skipped groups + begin of new group.
        while (groups_begin.size() <= group_index_retriever(vec[0]))
        {
            create_range_check(groups_begin.size(), num_groups);
            groups_begin.push_back(0);
        }

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
                    create_range_check(groups_begin.size(), num_groups);
                    groups_begin.push_back(i);
                }
            }
        }
    }

    // Write begin of remaining groups + end of last group.
    create_range_check(groups_begin.size(), num_groups);
    while (groups_begin.size() <= num_groups)
    {
        groups_begin.push_back(vec.size());
    }

    return IndexGroupedVector<T>(std::move(vec), std::move(groups_begin));
}

template<typename T>
template<typename GroupBoundaryChecker>
requires IsGroupBoundaryChecker<GroupBoundaryChecker, T>
inline IndexGroupedVector<T> IndexGroupedVector<T>::create(std::vector<std::remove_const_t<T>> vec, GroupBoundaryChecker group_boundary_checker)
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
IndexGroupedVector<T>::const_iterator::const_iterator() : m_pos(-1), m_parent(nullptr)
{
}

template<typename T>
IndexGroupedVector<T>::const_iterator::const_iterator(const IndexGroupedVector<T>& parent, bool begin) : m_pos(begin ? 0 : parent.size()), m_parent(&parent)
{
}

template<typename T>
IndexGroupedVector<T>::const_iterator::value_type IndexGroupedVector<T>::const_iterator::operator*() const
{
    return m_parent->at(m_pos);
}

template<typename T>
IndexGroupedVector<T>::const_iterator& IndexGroupedVector<T>::const_iterator::operator++()
{
    ++m_pos;
    return *this;
}

template<typename T>
IndexGroupedVector<T>::const_iterator IndexGroupedVector<T>::const_iterator::operator++(int)
{
    const_iterator tmp = *this;
    ++(*this);
    return tmp;
}

template<typename T>
bool IndexGroupedVector<T>::const_iterator::operator==(const const_iterator& other) const
{
    return (m_pos == other.m_pos);
}

template<typename T>
bool IndexGroupedVector<T>::const_iterator::operator!=(const const_iterator& other) const
{
    return !(*this == other);
}

template<typename T>
IndexGroupedVector<T>::const_iterator IndexGroupedVector<T>::begin() const
{
    return const_iterator(*this, true);
}

template<typename T>
IndexGroupedVector<T>::const_iterator IndexGroupedVector<T>::end() const
{
    return const_iterator(*this, false);
}

template<typename T>
std::span<T> IndexGroupedVector<T>::at(size_t pos)
{
    range_check(pos);
    return { m_vec.begin() + m_groups_begin.at(pos), m_vec.begin() + m_groups_begin.at(pos + 1) };
}

template<typename T>
const std::span<const T> IndexGroupedVector<T>::at(size_t pos) const
{
    range_check(pos);
    return { m_vec.begin() + m_groups_begin.at(pos), m_vec.begin() + m_groups_begin.at(pos + 1) };
}

template<typename T>
std::span<T> IndexGroupedVector<T>::operator[](size_t pos)
{
    return { m_vec.begin() + m_groups_begin[pos], m_vec.begin() + m_groups_begin[pos + 1] };
}

template<typename T>
const std::span<const T> IndexGroupedVector<T>::operator[](size_t pos) const
{
    return { m_vec.begin() + m_groups_begin[pos], m_vec.begin() + m_groups_begin[pos + 1] };
}

template<typename T>
std::span<T> IndexGroupedVector<T>::front()
{
    return this->at(0);
}

template<typename T>
const std::span<const T> IndexGroupedVector<T>::front() const
{
    return this->at(0);
}

template<typename T>
std::span<T> IndexGroupedVector<T>::back()
{
    return this->at(size() - 1);
}

template<typename T>
const std::span<const T> IndexGroupedVector<T>::back() const
{
    return this->at(size() - 1);
}

template<typename T>
size_t IndexGroupedVector<T>::size() const
{
    return m_groups_begin.size() - 1;
}

template<typename T>
std::vector<std::remove_const_t<T>>& IndexGroupedVector<T>::data()
{
    return m_vec;
}

template<typename T>
const std::vector<std::remove_const_t<T>>& IndexGroupedVector<T>::data() const
{
    return m_vec;
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
size_t IndexGroupedVectorBuilder<T>::start_group()
{
    m_groups_begin.push_back(m_vec.size());
    return m_vec.size();
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

template<typename T>
const std::vector<std::remove_const_t<T>>& IndexGroupedVectorBuilder<T>::data() const
{
    return m_vec;
}
}

#endif
