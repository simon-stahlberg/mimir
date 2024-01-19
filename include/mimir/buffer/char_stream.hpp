#ifndef MIMIR_BUFFER_CHAR_STREAM_HPP_
#define MIMIR_BUFFER_CHAR_STREAM_HPP_

#include <cstddef>
#include <vector>


namespace mimir::buffer {

class CharStream {
private:
    std::vector<char> m_data;

public:
    void write(const char* data, size_t amount) {
        m_data.insert(m_data.end(), data, data + amount);
    }

    void clear() { m_data.clear(); }

    const char* get_data() const { return m_data.data(); }

    size_t get_size() const { return m_data.size(); }
};

}  // namespace mimir

#endif // MIMIR_BUFFERS_CHAR_STREAM_HPP_
