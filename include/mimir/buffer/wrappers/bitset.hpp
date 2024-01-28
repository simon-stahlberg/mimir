#ifndef MIMIR_BUFFER_WRAPPERS_BITSET_HPP_
#define MIMIR_BUFFER_WRAPPERS_BITSET_HPP_

#include "../flatbuffers/bitset_generated.h"


namespace mimir {

/**
 * A builder for a flatbuffer BitsetFlat with the following table structure
 *
 *  table BitsetFlat {
 *    num_bits:uint32;
 *    segments:[uint64];
 *  }
*/
template<typename Block>
class BitsetBuilder {
private:
    uint32_t m_num_bits;
    std::vector<Block> m_data;

    static const Block default_value;

public:
    void set_num_bits(size_t num_bits) {
        m_num_bits = num_bits;
        m_data.resize(num_bits / (sizeof(Block) * 8) + 1, BitsetBuilder::default_value);
    }

    void reset() {
        // TODO
    }

    void set() {
        // TODO
    }

    void clear() {
        m_num_bits = 0;
        m_data.clear();
    }

    uint32_t get_num_bits() const { return m_num_bits; }
    const std::vector<Block>& get_data() const { return m_data; }
};

template<typename Block>
const Block BitsetBuilder<Block>::default_value = 0;


/**
 * A view on a flatbuffer BitsetFlat with the following table structure
 *
 *  table BitsetFlat {
 *    num_bits:uint32;
 *    segments:[uint64];
 *  }
*/
class BitsetView {
private:
    const BitsetFlat* m_flat;
public:
    explicit BitsetView(const BitsetFlat* flat)
        : m_flat(flat) { }

    // TODO: implement some bitset interface
};

}

#endif
