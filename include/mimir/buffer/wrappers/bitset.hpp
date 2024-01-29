#ifndef MIMIR_BUFFER_WRAPPERS_BITSET_HPP_
#define MIMIR_BUFFER_WRAPPERS_BITSET_HPP_

#include "../flatbuffers/bitset_generated.h"


namespace mimir
{

/**
 * A builder for a flatbuffer BitsetFlat with the following table structure
 *
 *  table BitsetFlat {
 *    num_bits:uint32;
 *    segments:[uint64];
 *  }
*/
template<typename Block = uint64_t>
class Bitset
{
private:
    std::vector<Block> data;
    bool default_bit_value = false;

    void resize_to_fit(const Bitset& other);

    static constexpr std::size_t block_size = sizeof(Block) * 8;
    static constexpr std::size_t block_zeroes = 0;
    static constexpr std::size_t block_ones = Block(-1);

public:
    Bitset() = default;
    // Initialize the bitset with a certain size
    Bitset(std::size_t size);

    Bitset(std::size_t size, bool default_bit_value);

    Bitset(const Bitset& other);

    Bitset(Bitset&& other) noexcept;

    Bitset& operator=(const Bitset& other);

    Bitset& operator=(Bitset&& other) noexcept;

    // Set a bit at a specific position
    void set(std::size_t position);

    // Set all bits
    void set();

    // Unset a bit at a specific position
    void unset(std::size_t position);

    // Unset all bits
    void unset();

    // Get the value of a bit at a specific position
    bool get(std::size_t position) const;

    // Find the next set bit, inclusive the given position
    std::size_t next_set_bit(std::size_t position) const;

    Bitset operator|(const Bitset& other) const;

    Bitset operator&(const Bitset& other) const;

    Bitset operator~() const;

    Bitset& operator|=(const Bitset& other);

    Bitset& operator&=(const Bitset& other);

    bool operator<(const Bitset& other) const;

    bool operator==(const Bitset& other) const;

    static constexpr std::size_t no_position = std::size_t(-1);

    template<typename T>
    friend class std::hash;

    // TODO (Dominik): I added this functionality:
    const std::vector<Block>& get_data() const { return data; }
    size_t get_size() const { return data.size(); }
    void clear() { data.clear(); }
};


/**
 * A view on a flatbuffer BitsetFlat with the following table structure
 *
 *  table BitsetFlat {
 *    num_bits:uint32;
 *    segments:[uint64];
 *  }
*/
class BitsetView
{
private:
    const BitsetFlat* m_flat;
public:
    explicit BitsetView(const BitsetFlat* flat)
        : m_flat(flat) { }

    // TODO: implement some bitset interface

    bool operator==(const BitsetView& other) const {
        // TODO: implement
        return false;
    }

    size_t hash() const {
        // TODO: implement
        return 0;
    }
};

class ConstBitsetView {

};


// Bitset1 is always a builder, Bitset2 is either Builder or View or ConstView
template<typename Bitset1, typename Bitset2>
class BitsetOperator {
public:
    static Bitset1& OrEquals(Bitset1& left, const Bitset2& right);
    static Bitset1 Or(const Bitset1& left, const Bitset2& right);
};

}

#endif
