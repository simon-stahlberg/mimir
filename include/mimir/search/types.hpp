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

/* ObjectList */
using ObjectListLayout = flatmemory::Vector<mimir::Object>;
using ObjectListBuilder = flatmemory::Builder<ObjectListLayout>;
using ObjectList = flatmemory::ConstView<ObjectListLayout>;
}

#endif
