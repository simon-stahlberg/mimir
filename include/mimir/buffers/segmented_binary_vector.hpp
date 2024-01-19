#ifndef MIMIR_SEARCH_SEARCH_NODE_HPP_
#define MIMIR_SEARCH_SEARCH_NODE_HPP_

#include <cstdint>
#include <cstddef>
#include <vector>


namespace mimir {

/// @brief Segmented vector of binary data.
///        The data is persistent and references will stay valid after reallocation.
/// @tparam N the amount of bytes per segments. 
///         Reasonable numbers are 10000-100000.
template<size_t N>
class SegmentedBinaryVector {
private:
    std::vector<std::vector<uint8_t>> m_segments;
    std::vector<uint32_t> m_begin_by_index;

    int size;
    int capacity;

    void increase_capacity();

public:
    uint8_t* push_back(uint8_t* data, int size);

    uint8_t* operator[](uint32_t index);
};

}

#endif 