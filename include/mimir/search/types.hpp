#ifndef MIMIR_SEARCH_TYPES_HPP_
#define MIMIR_SEARCH_TYPES_HPP_

#include "mimir/formalism/declarations.hpp"

#include <flatmemory/flatmemory.hpp>

namespace flat
{
/* Bitset */
using BitsetLayout = flatmemory::Bitset<uint64_t>;
using BitsetBuilder = flatmemory::Builder<BitsetLayout>;
using Bitset = flatmemory::ConstView<BitsetLayout>;

using BitsetVectorLayout = flatmemory::Vector<flatmemory::Bitset<uint64_t>>;
using BitsetVectorBuilder = flatmemory::Builder<BitsetVectorLayout>;
using BitsetVector = flatmemory::ConstView<BitsetVectorLayout>;

/* IntVector*/

using Int32tVectorLayout = flatmemory::Vector<int32_t>;
using Int32tVectorBuilder = flatmemory::Builder<Int32tVectorLayout>;
using Int32tVector = flatmemory::ConstView<Int32tVectorLayout>;

/* ObjectList */
using ObjectListLayout = flatmemory::Vector<mimir::Object>;
using ObjectListBuilder = flatmemory::Builder<ObjectListLayout>;
using ObjectList = flatmemory::ConstView<ObjectListLayout>;
}

#endif
