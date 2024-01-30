#ifndef MIMIR_BUFFER_FLATBUFFERS_UTILS_HPP_
#define MIMIR_BUFFER_FLATBUFFERS_UTILS_HPP_

#include "flatbuffers/bitset_generated.h"
#include "wrappers/bitset.hpp"

namespace mimir
{

template<typename Block>
flatbuffers::Offset<mimir::BitsetFlat>
serialize(Bitset<Block> bitset, flatbuffers::FlatBufferBuilder& ref_builder)
{
    return CreateBitsetFlat(
        ref_builder,
        bitset.get_default_bit_value(),
        ref_builder.CreateVector(bitset.get_data()));
}

}

#endif
