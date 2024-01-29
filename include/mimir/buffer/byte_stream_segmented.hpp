#ifndef MIMIR_BUFFER_BYTE_STREAM_SEGMENTED_HPP_
#define MIMIR_BUFFER_BYTE_STREAM_SEGMENTED_HPP_

#include <cassert>
#include <cstddef>
#include <vector>


namespace mimir
{

/// @tparam N the amount of bytes per segments.
///         Reasonable numbers are 10000-100000.
template<size_t N>
class ByteStreamSegmented
{
private:
    std::vector<uint8_t*> m_segments;

    size_t cur_segment_id;
    size_t cur_segment_pos;

    size_t size;
    size_t capacity;

    size_t last_written;

    /// @brief Allocate a block of size N and update tracking variables.
    void increase_capacity() {
        if (cur_segment_id == (m_segments.size() - 1)) {
            m_segments.push_back(new uint8_t[N]);
            cur_segment_pos = 0;
            capacity += N;
        }
        ++cur_segment_id;
        assert(cur_segment_id < m_segments.size());
    }

public:
    ByteStreamSegmented()
        : cur_segment_id(-1)
        , cur_segment_pos(0)
        , size(0)
        , capacity(0)
        , last_written(0) {
        // allocate first block of memory
        increase_capacity();
        assert(cur_segment_pos == 0);
        assert(cur_segment_id == 0);
    }
    ~ByteStreamSegmented() {
        for (uint8_t* ptr : m_segments) {
            delete[] ptr;
        }
    }
    ByteStreamSegmented(const ByteStreamSegmented& other) = delete;
    ByteStreamSegmented& operator=(const ByteStreamSegmented& other) = delete;
    ByteStreamSegmented(ByteStreamSegmented&& other) = default;
    ByteStreamSegmented& operator=(ByteStreamSegmented&& other) = default;

    /// @brief Write the data starting from the cur_segment_pos
    ///        in the segment with cur_segment_id, if it fits,
    ///        and otherwise, push_back a new segment first.
    uint8_t* write(const uint8_t* data, size_t amount) {
        assert(amount <= N);
        if (amount > (N - cur_segment_pos)) {
            increase_capacity();
        }
        uint8_t* result_data = &m_segments[cur_segment_id][cur_segment_pos];
        memcpy(result_data, data, amount);
        cur_segment_pos += amount;
        size += amount;
        last_written += amount;
        return result_data;
    }

    /// @brief Undo the last write operation.
    void undo_last_write() {
        cur_segment_pos -= last_written;
        last_written = 0;
    }

    /// @brief Set the write head to the beginning.
    void clear() {
        cur_segment_id = 0;
        cur_segment_pos = 0;
        size = 0;
        last_written = 0;
    }

    [[nodiscard]] size_t get_size() const { return size; }
    [[nodiscard]] size_t get_capacity() const { return capacity; }
};

}

#endif
