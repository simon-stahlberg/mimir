#ifndef MIMIR_BUFFER_BYTE_STREAM_HPP_
#define MIMIR_BUFFER_BYTE_STREAM_HPP_

#include "byte_stream_utils.hpp"

#include <cstdint>
#include <cstddef>
#include <vector>
#include <iostream>


namespace mimir {


class ByteStream {
private:
    std::vector<uint8_t> m_data;

public:
    void write(const uint8_t* data, size_t amount) {
        m_data.insert(m_data.end(), data, data + amount);
    }
    
    /// @brief Writes a value to the stream.
    template<TriviallyCopyable T>
    size_t write(const T& value) {
        write(reinterpret_cast<const uint8_t*>(&value), sizeof(value));
        return sizeof(value);
    }

    /// @brief Writes a pointer to the stream
    template<typename T>
    size_t write(const T* pointer) {
        uintptr_t address = reinterpret_cast<uintptr_t>(pointer);
        write(reinterpret_cast<const uint8_t*>(&address), sizeof(address));
        return sizeof(address);
    }

    size_t write_padding(size_t num_bytes) {
        m_data.insert(m_data.end(), num_bytes, 0);
        return num_bytes;
    }

    void clear() { m_data.clear(); }

    [[nodiscard]] uint8_t* get_data() { return m_data.data(); }
    [[nodiscard]] const uint8_t* get_data() const { return m_data.data(); }

    [[nodiscard]] size_t get_size() const { return m_data.size(); }
    [[nodiscard]] size_t get_capacity() const { return m_data.capacity(); }
};

}  // namespace mimir

#endif // MIMIR_BUFFER_BYTE_STREAM_HPP_