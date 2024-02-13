#ifndef MIMIR_SEARCH_TYPES_HPP_
#define MIMIR_SEARCH_TYPES_HPP_

#include <flatmemory/flatmemory.hpp>


namespace mimir
{
    using BitsetLayout    = flatmemory::Bitset<uint64_t>;

    using Bitset          = flatmemory::Builder<BitsetLayout>;
    using ConstBitsetView = flatmemory::ConstView<BitsetLayout>;
}

#endif 
