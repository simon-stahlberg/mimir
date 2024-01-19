
#ifndef MIMIR_BUFFER_VIEW_BASE_HPP_
#define MIMIR_BUFFER_VIEW_BASE_HPP_

#include "../algorithms/murmurhash3.hpp"

#include <cstring> // For std::memcmp
#include <functional>


namespace mimir::buffer {

/// @brief User must provide a fully instantiated template.
template<typename T>
class View {
private:
    // Basic meta data that every view must contain to be equality comparable and hashable
    char* m_data;
    size_t m_size;

public:
    View(char* data, size_t size) : m_data(data), m_size(size) { }

    bool operator==(const View& other) const {
        if (get_size() != other.get_size()) return false;
        return (std::memcmp(get_data(), other.get_data(), get_size()) == 0);
    }

    char* get_data() { return m_data; }
    const char* get_data() const { return m_data; }
    size_t get_size() const { return m_size; }
};

}

namespace std {
    template<typename T>
    struct hash<mimir::buffer::View<T>>
    {
        std::size_t operator()(const mimir::buffer::View<T>& view) const {
            size_t seed = view.get_size();
            int64_t hash[2];
            MurmurHash3_x64_128(view.get_data(), view.get_size(), seed, hash);
            return static_cast<std::size_t>(hash[0] + 0x9e3779b9 + (hash[1] << 6) + (hash[1] >> 2));
        }
    };
}


#endif
