#ifndef MIMIR_BUFFER_BYTE_STREAM_HPP_
#define MIMIR_BUFFER_BYTE_STREAM_HPP_

#include "../common/mixins.hpp"

#include <cstddef>
#include <vector>
#include <iostream>
#include <type_traits>


namespace mimir {

/**
 * Ensures that we only serialize types that are trivially copieable.
 * Trivially copieable types are int float double ..
 * Untrivially copieable types are std::vector, std::unordered_set, ...
 * For untrivially copieable types we will provide our own logic.
*/
template<typename T>
concept TriviallyCopyable = std::is_trivially_copyable_v<T>;


class ByteStream : public UncopyableMixin<ByteStream> {
private:
    std::vector<char> m_data;

    void write(const char* data, size_t amount) {
        m_data.insert(m_data.end(), data, data + amount);
    }

public:
    /// @brief Writes a value to the stream.
    template<TriviallyCopyable T>
    void write(const T& value) {
        write(reinterpret_cast<const char*>(&value), sizeof(value));
    }

    /// @brief Writes a pointer to the stream
    template<typename T>
    void write(const T* pointer) {
        uintptr_t address = reinterpret_cast<uintptr_t>(pointer);
        write(reinterpret_cast<const char*>(&address), sizeof(address));
    }

    void clear() { m_data.clear(); }

    [[nodiscard]] char* get_data() { return m_data.data(); }
    [[nodiscard]] const char* get_data() const { return m_data.data(); }

    [[nodiscard]] size_t get_size() const { return m_data.size(); }
    [[nodiscard]] size_t get_capacity() const { return m_data.capacity(); }
};

}  // namespace mimir

#endif // MIMIR_BUFFER_BYTE_STREAM_HPP_
