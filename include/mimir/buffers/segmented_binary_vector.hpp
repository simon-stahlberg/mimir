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

    size_t size;
    size_t capacity;

    void increase_capacity();

public:
    /// @brief 
    /// @param data 
    /// @param size 
    /// @return the pointer to the position at which the data was inserted
    char* push_back(const char* data, int size) {
        // TODO (Dominik): implement
        return nullptr;
    }
};

}  // namespace mimir

#endif // MIMIR_BUFFERS_SEGMENTED_BINARY_VECTOR_HPP_
