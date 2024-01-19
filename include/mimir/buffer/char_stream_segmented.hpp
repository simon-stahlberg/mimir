#ifndef MIMIR_BUFFER_CHAR_STREAM_SEGMENTED_HPP_
#define MIMIR_BUFFER_CHAR_STREAM_SEGMENTED_HPP_

#include "../common/mixins.hpp"

#include <cassert>
#include <cstddef>
#include <vector>


namespace mimir {

/// @tparam N the amount of bytes per segments.
///         Reasonable numbers are 10000-100000.
template<size_t N>
class CharStreamSegmented : public UncopyableMixin<CharStreamSegmented<N>> {
private:
    std::vector<std::vector<char>> m_segments;

    size_t cur_segment_id;
    size_t cur_segment_pos;

    size_t size;
    size_t capacity;

    void increase_capacity() {
        m_segments.push_back(std::vector<char>(N));
        ++cur_segment_id;
        cur_segment_pos = 0;
        capacity += N;
    }

public:
    /// @brief Write the data starting from the cur_segment_pos
    ///        in the segment with cur_segment_id, if it fits,
    ///        and otherwise, push_back a new segment first.
    /// @param data
    /// @param size
    /// @return
    char* write(const char* data, size_t amount) {
        if (amount > (N - m_segments.back().size())) {
            increase_capacity();
        }
        auto& cur_segment = m_segments.back();
        char* result_data = cur_segment.end().base();
        cur_segment.insert(cur_segment.end(), data, data + amount);
        size += amount;
        return result_data;
    }

    /// @brief Set the write head to the beginning.
    void clear() {
        cur_segment_id = 0;
        cur_segment_pos = 0;
    }

    const char* get_data(size_t segment_id) const {
        assert(segment_id <= m_segments.size());
        m_segments[segment_id].data();
    }
    size_t get_size(size_t segment_id) const {
        assert(segment_id <= m_segments.size());
        return m_segments[segment_id].size();
    }

    size_t get_size() const { return size; }
};

}  // namespace mimir

#endif // MIMIR_BUFFERS_SEGMENTED_BINARY_VECTOR_HPP_
