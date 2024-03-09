#ifndef MIMIR_SEARCH_TYPES_HPP_
#define MIMIR_SEARCH_TYPES_HPP_

#include "mimir/formalism/declarations.hpp"

#include <flatmemory/flatmemory.hpp>

namespace mimir
{
/* Bitset */
using BitsetLayout = flatmemory::Bitset<uint64_t>;
using BitsetBuilder = flatmemory::Builder<BitsetLayout>;
using ConstBitsetView = flatmemory::ConstView<BitsetLayout>;

/* ObjectList */
using ObjectListLayout = flatmemory::Vector<Object>;
using ObjectListBuilder = flatmemory::Builder<ObjectListLayout>;
using ConstObjectListView = flatmemory::ConstView<ObjectListLayout>;
}

#endif
