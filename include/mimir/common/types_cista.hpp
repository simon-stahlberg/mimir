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
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef MIMIR_COMMON_TYPES_CISTA_HPP_
#define MIMIR_COMMON_TYPES_CISTA_HPP_

#include "cista/containers/dynamic_bitset.h"
#include "cista/containers/external_ptr.h"
#include "cista/containers/flexible_delta_index_vector.h"
#include "cista/containers/flexible_index_vector.h"
#include "cista/containers/vector.h"
#include "mimir/buffering/cista_unordered_map.hpp"
#include "mimir/buffering/cista_unordered_set.hpp"
#include "mimir/common/declarations.hpp"
#include "mimir/common/hash.hpp"

#include <algorithm>
#include <ostream>

namespace mimir
{

/**
 * Cista
 */

using FlatBitset = cista::offset::dynamic_bitset<uint64_t>;
using FlatIndexList = cista::offset::flexible_index_vector<Index>;
using FlatDoubleList = cista::offset::vector<double>;

using FlatIndexListMap = mimir::buffering::CistaUnorderedMap<FlatIndexList, Index>;
using FlatDoubleListMap = mimir::buffering::CistaUnorderedMap<FlatDoubleList, Index>;

template<typename... Ts>
using FlatBitsets = boost::hana::map<boost::hana::pair<boost::hana::type<Ts>, FlatBitset>...>;
template<typename... Ts>
using FlatIndexLists = boost::hana::map<boost::hana::pair<boost::hana::type<Ts>, FlatIndexList>...>;
template<typename... Ts>
using FlatDoubleLists = boost::hana::map<boost::hana::pair<boost::hana::type<Ts>, FlatDoubleList>...>;

/**
 * Forward Declarations
 */

inline std::ostream& operator<<(std::ostream& os, const FlatBitset& set);
inline std::ostream& operator<<(std::ostream& os, const FlatIndexList& list);
template<typename T,
         template<typename>
         typename Ptr,
         bool IndexPointers = false,
         typename TemplateSizeType = std::uint32_t,
         class Allocator = cista::allocator<T, Ptr>>
inline std::ostream& operator<<(std::ostream& os, const cista::basic_vector<T, Ptr, IndexPointers, TemplateSizeType, Allocator>& list);

inline std::ostream& operator<<(std::ostream& os, const FlatBitset& set)
{
    os << "[";
    size_t i = 0;
    for (const auto& element : set)
    {
        if (i != 0)
            os << ", ";
        os << element;
        ++i;
    }
    os << "]";
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const FlatIndexList& list)
{
    os << "[";
    size_t i = 0;
    if (list.is_compressed())
    {
        for (const auto& element : list.compressed_range())
        {
            if (i != 0)
                os << ", ";
            os << element;
            ++i;
        }
    }
    else
    {
        for (const auto& element : list.uncompressed_range())
        {
            if (i != 0)
                os << ", ";
            os << element;
            ++i;
        }
    }
    os << "]";
    return os;
}

/* BasicVector */

template<typename T, template<typename> typename Ptr, bool IndexPointers, typename TemplateSizeType, class Allocator>
inline std::ostream& operator<<(std::ostream& os, const cista::basic_vector<T, Ptr, IndexPointers, TemplateSizeType, Allocator>& list)
{
    os << "[";
    size_t i = 0;
    for (const auto& element : list)
    {
        if (i != 0)
            os << ", ";
        os << element;
        ++i;
    }
    os << "]";
    return os;
}

/// @brief
/// We use this to test applicability of sparse actions/axioms in dense intermediate states.
/// We could get rid of this if we avoid translation to dense state representation.
/// @param bitset
/// @param range
/// @return
template<std::ranges::input_range Range>
    requires IsRangeOver<Range, Index>
inline bool are_disjoint(const FlatBitset& bitset, const Range& range)
{
    for (const auto index : range)
    {
        if (bitset.get(index))
        {
            return false;
        }
    }
    return true;
}

template<std::ranges::input_range Range1, std::ranges::input_range Range2>
    requires IsRangeOver<Range1, Index> && IsRangeOver<Range2, Index>
inline bool are_disjoint(const Range1& lhs, const Range2& rhs)
{
    assert(std::is_sorted(lhs.begin(), lhs.end()));
    assert(std::is_sorted(rhs.begin(), rhs.end()));

    // Use two iterators to traverse both vectors simultaneously
    auto it1 = lhs.begin();
    auto it2 = rhs.begin();

    while (it1 != lhs.end() && it2 != rhs.end())
    {
        if (*it1 < *it2)
        {
            ++it1;
        }
        else if (*it2 < *it1)
        {
            ++it2;
        }
        else
        {
            // Common element found
            return false;
        }
    }

    return true;
}

/// @brief
/// We use this to test applicability of sparse actions/axioms in dense intermediate states.
/// We could get rid of this if we avoid translation to dense state representation.
/// @param bitset
/// @param range
/// @return
template<std::ranges::input_range Range>
    requires IsRangeOver<Range, Index>
inline bool is_supseteq(const FlatBitset& bitset, const Range& range)
{
    for (const auto& index : range)
    {
        if (!bitset.get(index))
        {
            return false;
        }
    }
    return true;
}

/// @brief
/// We use this to test applicability of actions/axioms in fully sparse representations.
/// @param lhs
/// @param rhs
/// @return
template<std::ranges::input_range Range1, std::ranges::input_range Range2>
    requires IsRangeOver<Range1, Index> && IsRangeOver<Range2, Index>
inline bool is_supseteq(const Range1& lhs, const Range2& rhs)
{
    assert(std::is_sorted(lhs.begin(), lhs.end()));
    assert(std::is_sorted(rhs.begin(), rhs.end()));

    return std::includes(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

/// @brief Count the number of elements in `lhs` that are not in `rhs`.
/// @param lhs
/// @param rhs
/// @return
template<std::ranges::input_range Range1, std::ranges::input_range Range2>
    requires IsRangeOver<Range1, Index> && IsRangeOver<Range2, Index>
inline size_t count_set_difference(const Range1& lhs, const Range2& rhs)
{
    assert(std::is_sorted(lhs.begin(), lhs.end()));
    assert(std::is_sorted(rhs.begin(), rhs.end()));

    size_t result = 0;

    // Use two iterators to traverse both vectors simultaneously
    auto it1 = lhs.begin();
    auto it2 = rhs.begin();

    while (it1 != lhs.end() && it2 != rhs.end())
    {
        if (*it1 < *it2)
        {
            ++it1;
            ++result;
        }
        else if (*it2 < *it1)
        {
            ++it2;
        }
        else
        {
            // Common element found
            ++it1;
            ++it2;
        }
    }

    result += std::distance(it1, lhs.end());

    return result;
}

/// @brief Count the number of elements in `lhs` that are in `rhs`.
/// @param lhs
/// @param rhs
/// @return
template<std::ranges::input_range Range1, std::ranges::input_range Range2>
    requires IsRangeOver<Range1, Index> && IsRangeOver<Range2, Index>
inline size_t count_set_intersection(const Range1& lhs, const Range2& rhs)
{
    assert(std::is_sorted(lhs.begin(), lhs.end()));
    assert(std::is_sorted(rhs.begin(), rhs.end()));

    size_t result = 0;

    // Use two iterators to traverse both vectors simultaneously
    auto it1 = lhs.begin();
    auto it2 = rhs.begin();

    while (it1 != lhs.end() && it2 != rhs.end())
    {
        if (*it1 < *it2)
        {
            ++it1;
        }
        else if (*it2 < *it1)
        {
            ++it2;
        }
        else
        {
            // Common element found
            ++it1;
            ++it2;
            ++result;
        }
    }

    return result;
}

template<std::ranges::input_range Range>
    requires IsConvertibleRangeOver<Range, Index>
void insert_into_bitset(const Range& range, FlatBitset& ref_bitset)
{
    for (const auto& element : range)
    {
        ref_bitset.set(element);
    }
}

template<std::ranges::input_range Range>
    requires IsConvertibleRangeOver<Range, Index>
void insert_into_vector(const Range& range, FlatIndexList& ref_vec)
{
    for (const auto& element : range)
    {
        ref_vec.push_back(element);
    }
}

}

#endif
