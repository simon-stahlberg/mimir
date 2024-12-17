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
#include "cista/containers/flexible_index_vector.h"
#include "cista/containers/vector.h"
#include "mimir/common/concepts.hpp"
#include "mimir/common/types.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/formalism/predicate_tag.hpp"

#include <algorithm>
#include <ostream>

namespace mimir
{
/* Bitset */

using FlatBitset = cista::offset::dynamic_bitset<uint64_t>;

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

/* IndexList */

using FlatIndexList = cista::offset::flexible_index_vector<Index>;
// using FlatIndexList = cista::offset::vector<Index>;

inline std::ostream& operator<<(std::ostream& os, const FlatIndexList& set)
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

inline bool contains(const FlatIndexList& vec, Index value) { return std::find(vec.begin(), vec.end(), value) != vec.end(); }

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

inline bool is_supseteq(const FlatIndexList& vec1, const FlatIndexList& vec2)
{
    assert(std::is_sorted(vec1.begin(), vec1.end()));
    assert(std::is_sorted(vec2.begin(), vec2.end()));

    return std::includes(vec1.begin(), vec1.end(), vec2.begin(), vec2.end());
}

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
