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
    std::vector<Block> m_data;
    bool m_default_bit_value = false;

    void resize_to_fit(const Bitset& other) {
        if (m_data.size() < other.m_data.size())
        {
            m_data.resize(other.m_data.size(), m_default_bit_value ? block_ones : block_zeroes);
        }
    }

    static constexpr std::size_t block_size = sizeof(Block) * 8;
    static constexpr Block block_zeroes = 0;
    static constexpr Block block_ones = Block(-1);

public:
    Bitset() = default;
    // Initialize the bitset with a certain size
    Bitset(std::size_t size) : m_data(size / (sizeof(Block) * 8) + 1, block_zeroes) {}

    Bitset(std::size_t size, bool default_bit_value):
        m_data(size / (sizeof(Block) * 8) + 1, m_default_bit_value ? block_ones : block_zeroes),
        m_default_bit_value(m_default_bit_value) { }

    Bitset(const Bitset& other) = default;
    Bitset& operator=(const Bitset& other) = default;
    Bitset(Bitset&& other) noexcept = default;
    Bitset& operator=(Bitset&& other) noexcept = default;

    // Set a bit at a specific position
    void set(std::size_t position)
    {
        const std::size_t index = position / block_size;   // Find the index in the vector
        const std::size_t offset = position % block_size;  // Find the offset within the std::size_t

        if (index >= m_data.size())
        {
            m_data.resize(index + 1, m_default_bit_value ? block_ones : block_zeroes);
        }

        m_data[index] |= (static_cast<Block>(1) << offset);  // Set the bit at the offset
    }

    // Unset a bit at a specific position
    void unset(std::size_t position)
    {
        const std::size_t index = position / block_size;   // Find the index in the vector
        const std::size_t offset = position % block_size;  // Find the offset within the std::size_t

        if (index >= m_data.size())
        {
            m_data.resize(index + 1, m_default_bit_value ? block_ones : block_zeroes);
        }

        m_data[index] &= ~(static_cast<Block>(1) << offset);  // Set the bit at the offset
    }

    void unset_all()
    {

    }

    void unset_all(bool default_bit_value)
    {

    }

    // Get the value of a bit at a specific position
    bool get(std::size_t position) const {
        {
        const std::size_t index = position / block_size;

        if (index < m_data.size())
        {
            const std::size_t offset = position % block_size;
            return (m_data[index] & (static_cast<Block>(1) << offset)) != 0;
        }
        else
        {
            return m_default_bit_value;
        }
    }
    }

    // Find the next set bit, inclusive the given position
    std::size_t next_set_bit(std::size_t position) const
    {
        std::size_t index = position / block_size;
        std::size_t offset = position % block_size;

        while (index < m_data.size())
        {
            // Shift so that we start checking from the offset
            const Block value = m_data[index] >> offset;

            if (value)
            {
                // If there are set bits in the current value
                const auto lsb_position = get_lsb_position(value);
                return index * block_size + offset + lsb_position;
            }

            // Reset offset for the next value
            offset = 0;
            index++;
        }

        return no_position;
    }

    Bitset operator|(const Bitset& other) const
    {
        Bitset result = *this;
        result.resize_to_fit(other);

        for (std::size_t i = 0; i < other.m_data.size(); ++i)
        {
            result.m_data[i] |= other.m_data[i];
        }

        return result;
    }

    Bitset operator&(const Bitset& other) const
    {
        Bitset result = *this;
        result.resize_to_fit(other);

        for (std::size_t i = 0; i < other.m_data.size(); ++i)
        {
            result.m_data[i] &= other.m_data[i];
        }

        return result;
    }

    Bitset operator~() const
    {
        Bitset result = *this;
        result.m_default_bit_value = !m_default_bit_value;

        for (Block& value : result.m_data)
        {
            value = ~value;
        }

        return result;
    }

    Bitset& operator|=(const Bitset& other)
    {
        this->resize_to_fit(other);

        for (std::size_t i = 0; i < other.m_data.size(); ++i)
        {
            this->m_data[i] |= other.m_data[i];
        }

        return *this;
    }

    Bitset& operator&=(const Bitset& other)
    {
        this->resize_to_fit(other);

        for (std::size_t i = 0; i < other.m_data.size(); ++i)
        {
            this->m_data[i] &= other.m_data[i];
        }

        return *this;
    }

    bool operator<(const Bitset& other) const
    {
        std::size_t common_size = std::min(m_data.size(), other.m_data.size());

        for (std::size_t index = 0; index < common_size; ++index)
        {
            if (m_data[index] < other.m_data[index])
            {
                return true;
            }
        }

        std::size_t max_size = std::max(m_data.size(), other.m_data.size());

        for (std::size_t index = common_size; index < max_size; ++index)
        {
            std::size_t this_value = index < m_data.size() ? m_data[index] : (m_default_bit_value ? block_ones : block_zeroes);
            std::size_t other_value = index < other.m_data.size() ? other.m_data[index] : (other.m_default_bit_value ? block_ones : block_zeroes);

            if (this_value < other_value)
            {
                return true;
            }
        }

        return false;
    }

    bool operator==(const Bitset& other) const
    {
        std::size_t common_size = std::min(m_data.size(), other.m_data.size());

        for (std::size_t index = 0; index < common_size; ++index)
        {
            if (m_data[index] != other.m_data[index])
            {
                return false;
            }
        }

        std::size_t max_size = std::max(m_data.size(), other.m_data.size());

        for (std::size_t index = common_size; index < max_size; ++index)
        {
            std::size_t this_value = index < m_data.size() ? m_data[index] : (m_default_bit_value ? block_ones : block_zeroes);
            std::size_t other_value = index < other.m_data.size() ? other.m_data[index] : (other.m_default_bit_value ? block_ones : block_zeroes);

            if (this_value != other_value)
            {
                return false;
            }
        }

        return true;
    }

    static constexpr std::size_t no_position = std::size_t(-1);

    template<typename T>
    friend class std::hash;

    /// @brief Get m_data for serialization
    bool get_default_bit_value() const { return m_default_bit_value; }
    const std::vector<Block>& get_data() const { return m_data; }
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
