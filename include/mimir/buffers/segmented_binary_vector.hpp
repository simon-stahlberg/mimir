#ifndef MIMIR_BUFFERS_SEGMENTED_BINARY_VECTOR_HPP_
#define MIMIR_BUFFERS_SEGMENTED_BINARY_VECTOR_HPP_

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
    std::vector<std::vector<char>> m_segments;

    int size;
    int capacity;

    void increase_capacity();

public:
    char* push_back(char* data, int size);
};

}  // namespace mimir

#endif // MIMIR_BUFFERS_SEGMENTED_BINARY_VECTOR_HPP_
