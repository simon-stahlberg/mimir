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

#include "mimir/formalism/declarations.hpp"

#include <flatmemory/flatmemory.hpp>

namespace mimir
{
/* Bitset */

using FlatBitsetLayout = flatmemory::Bitset<uint64_t>;
using FlatBitsetBuilder = flatmemory::Builder<FlatBitsetLayout>;
using FlatBitset = flatmemory::ConstView<FlatBitsetLayout>;

using FlatBitsetVectorLayout = flatmemory::Vector<flatmemory::Bitset<uint64_t>>;
using FlatBitsetVectorBuilder = flatmemory::Builder<FlatBitsetVectorLayout>;
using FlatBitsetVector = flatmemory::ConstView<FlatBitsetVectorLayout>;

/* ObjectList */

using FlatObjectListLayout = flatmemory::Vector<Object>;
using FlatObjectListBuilder = flatmemory::Builder<FlatObjectListLayout>;
using FlatObjectList = flatmemory::ConstView<FlatObjectListLayout>;
}

#endif
