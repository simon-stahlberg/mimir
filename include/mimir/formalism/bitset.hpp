#ifndef MIMIR_FORMALISM_BITSET_HPP_
#define MIMIR_FORMALISM_BITSET_HPP_

#include <algorithm>
#include <vector>

namespace mimir::formalism
{
    class Bitset
    {
      private:
        std::vector<std::size_t> data;
        bool default_bit_value = false;

        void resize_to_fit(const Bitset& other);

        static constexpr std::size_t block_size = sizeof(std::size_t) * 8;
        static constexpr std::size_t block_zeroes = 0;
        static constexpr std::size_t block_ones = std::size_t(-1);

      public:
        // Initialize the bitset with a certain size
        Bitset(std::size_t size);

        Bitset(std::size_t size, bool default_bit_value);

        Bitset(const Bitset& other);

        Bitset(Bitset&& other) noexcept;

        Bitset& operator=(const Bitset& other);

        Bitset& operator=(Bitset&& other) noexcept;

        // Set a bit at a specific position
        void set(std::size_t position);

        // Unset a bit at a specific position
        void unset(std::size_t position);

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
    };

}  // namespace formalism

namespace std
{
    // Inject comparison and hash functions to make pointers behave appropriately with ordered and unordered datastructures
    template<>
    struct hash<mimir::formalism::Bitset>
    {
        std::size_t operator()(const mimir::formalism::Bitset& bitset) const;
    };

    template<>
    struct less<mimir::formalism::Bitset>
    {
        bool operator()(const mimir::formalism::Bitset& left_bitset, const mimir::formalism::Bitset& right_bitset) const;
    };

    template<>
    struct equal_to<mimir::formalism::Bitset>
    {
        bool operator()(const mimir::formalism::Bitset& left_bitset, const mimir::formalism::Bitset& right_bitset) const;
    };
}  // namespace std

#endif
