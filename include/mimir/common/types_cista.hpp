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
#include "cista/containers/flexible_index_vector.h"
#include "cista/containers/vector.h"
#include "mimir/buffering/unordered_set.h"
#include "mimir/common/concepts.hpp"
#include "mimir/common/hash.hpp"
#include "mimir/common/types.hpp"

#include <algorithm>
#include <ostream>

namespace mimir
{

/* Bitset */

using FlatBitset = cista::offset::dynamic_bitset<uint64_t>;
/* IndexList */
using FlatIndexList = cista::offset::flexible_index_vector<Index>;
using FlatIndexListSet = mimir::buffering::UnorderedSet<FlatIndexList>;
/* ExternalPtr */
template<typename T>
using FlatExternalPtr = cista::offset::external_ptr<T>;
template<typename T>
using FlatExternalPtrList = cista::offset::vector<cista::offset::external_ptr<T>>;
/* DoubleList */
using FlatDoubleList = cista::offset::vector<double>;
using FlatDoubleListSet = mimir::buffering::UnorderedSet<cista::offset::vector<double>>;

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
template<typename T>
std::ostream& operator<<(std::ostream& os, const FlatExternalPtr<T>& ptr);

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

template<typename T>
std::ostream& operator<<(std::ostream& os, const FlatExternalPtr<T>& ptr)
{
    os << *ptr;
    return os;
}

/// @brief Check whether `value` exists in the given `vec`.
/// Runs binary search to find a value in a vec.
/// @param vec is the vector.
/// @param value is the value.
/// @return true iff the value exists in the vector.
inline bool contains(const FlatIndexList& vec, Index value) { return std::find(vec.begin(), vec.end(), value) != vec.end(); }

/// @brief
/// We use this to test applicability of sparse actions/axioms in dense intermediate states.
/// We could get rid of this if we avoid translation to dense state representation.
/// @param bitset
/// @param list
/// @return
inline bool are_disjoint(const FlatBitset& bitset, const FlatIndexList& list)
{
    for (const auto index : list)
    {
        if (bitset.get(index))
        {
            return false;
        }
    }
    return true;
}

/// @brief
/// We use this to test applicability of sparse actions/axioms in dense intermediate states.
/// We could get rid of this if we avoid translation to dense state representation.
/// @param bitset
/// @param vec
/// @return
inline bool is_supseteq(const FlatBitset& bitset, const FlatIndexList& vec)
{
    for (const auto& index : vec)
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
/// @param vec1
/// @param vec2
/// @return
inline bool is_supseteq(const FlatIndexList& vec1, const FlatIndexList& vec2)
{
    assert(std::is_sorted(vec1.begin(), vec1.end()));
    assert(std::is_sorted(vec2.begin(), vec2.end()));

    return std::includes(vec1.begin(), vec1.end(), vec2.begin(), vec2.end());
}

/// @brief
/// We use this to test applicability of actions/axioms in fully sparse representations.
/// @param vec1
/// @param vec2
/// @return
inline bool are_disjoint(const FlatIndexList& vec1, const FlatIndexList& vec2)
{
    assert(std::is_sorted(vec1.begin(), vec1.end()));
    assert(std::is_sorted(vec2.begin(), vec2.end()));

    // Use two iterators to traverse both vectors simultaneously
    auto it1 = vec1.begin();
    auto it2 = vec2.begin();

    while (it1 != vec1.end() && it2 != vec2.end())
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

    // No common element found
    return true;
}

template<typename Range>
    requires IsConvertibleRangeOver<Range, Index>  //
void insert_into_bitset(const Range& range, FlatBitset& ref_bitset)
{
    for (const auto& element : range)
    {
        ref_bitset.set(element);
    }
}

template<typename Range>
    requires IsConvertibleRangeOver<Range, Index>  //
void insert_into_vector(const Range& range, FlatIndexList& ref_vec)
{
    for (const auto& element : range)
    {
        ref_vec.push_back(element);
    }
}

}

#endif
