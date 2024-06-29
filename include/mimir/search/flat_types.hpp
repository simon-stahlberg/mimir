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

#ifndef MIMIR_SEARCH_FLAT_TYPES_HPP_
#define MIMIR_SEARCH_FLAT_TYPES_HPP_

#include "mimir/formalism/object.hpp"
#include "mimir/formalism/predicate_category.hpp"

#include <flatmemory/flatmemory.hpp>
#include <ostream>

namespace mimir
{
/* Bitset */

// General purpose bitset
template<typename Tag = void>
using FlatBitsetLayout = flatmemory::Bitset<uint64_t, Tag>;
template<typename Tag = void>
using FlatBitsetBuilder = flatmemory::Builder<FlatBitsetLayout<Tag>>;
template<typename Tag = void>
using FlatBitset = flatmemory::ConstView<FlatBitsetLayout<Tag>>;

inline std::ostream& operator<<(std::ostream& os, const FlatBitset<>& set)
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

/* ObjectList */

using FlatObjectListLayout = flatmemory::Vector<Object>;
using FlatObjectListBuilder = flatmemory::Builder<FlatObjectListLayout>;
using FlatObjectList = flatmemory::ConstView<FlatObjectListLayout>;

/* IndexList */

using FlatIndexListLayout = flatmemory::Vector<uint32_t>;
using FlatIndexListBuilder = flatmemory::Builder<FlatIndexListLayout>;
using FlatIndexList = flatmemory::ConstView<FlatIndexListLayout>;

template<flatmemory::IsBitset Bitset>
bool are_disjoint(const Bitset& bitset, FlatIndexList list)
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

template<flatmemory::IsBitset Bitset>
bool is_superseteq(const Bitset& bitset, FlatIndexList list)
{
    for (const auto index : list)
    {
        if (!bitset.get(index))
        {
            return false;
        }
    }
    return true;
}

}

#endif
