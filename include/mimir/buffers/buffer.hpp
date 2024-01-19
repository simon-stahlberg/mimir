#ifndef MIMIR_BUFFERS_BUFFER_HPP_
#define MIMIR_BUFFERS_BUFFER_HPP_

#include <cstdint>
#include <cstddef>
#include <vector>


namespace mimir {

class BinaryBuffer {
private:
    std::vector<char> m_data;

public:
    void write(const char* data, int size) {
        m_data.insert(m_data.end(), data, data + size);
    }

    void clear() { m_data.clear(); }

    const char* get_data() const { return m_data.data(); }
    size_t get_size() const { return m_data.size(); }
};

}  // namespace mimir

#endif // MIMIR_BUFFERS_BUFFER_HPP_
