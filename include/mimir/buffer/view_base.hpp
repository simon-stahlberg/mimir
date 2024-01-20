
#ifndef MIMIR_BUFFER_VIEW_BASE_HPP_
#define MIMIR_BUFFER_VIEW_BASE_HPP_

#include "types.hpp"

#include "../algorithms/murmurhash3.hpp"

#include <cstring> // For std::memcmp
#include <functional>


namespace mimir {

/**
 * Interface class
*/
template<typename Derived>
class ViewBase {
private:
    friend Derived;

    // Basic meta data that every view must contain to be equality comparable and hashable
    char* m_data;

protected:
    explicit ViewBase(char* data) : m_data(data) { }

    /// @brief Access the data to be interpreted by derived classes.
    [[nodiscard]] char* get_data() { return m_data; }
    [[nodiscard]] const char* get_data() const { return m_data; }

    /// @brief The first 4 bytes are always reserved for the size.
    [[nodiscard]] DataSizeType get_size() const { return reinterpret_cast<DataSizeType>(m_data); }

public:
    [[nodiscard]] bool operator==(const ViewBase& other) const {
        if (get_size() != other.get_size()) return false;
        return (std::memcmp(get_data(), other.get_data(), get_size()) == 0);
    }
};


/**
 * Implementation class for some type T.
 * Provide an implementation for T by providing fully specified template.
*/
template<typename T>
class View : public ViewBase<View<T>> {};

}


namespace std {
    template<typename T>
    struct hash<mimir::View<T>>
    {
        std::size_t operator()(const mimir::View<T>& view) const {
            size_t seed = view.get_size();
            int64_t hash[2];
            MurmurHash3_x64_128(view.get_data(), view.get_size(), seed, hash);
            return static_cast<std::size_t>(hash[0] + 0x9e3779b9 + (hash[1] << 6) + (hash[1] >> 2));
        }
    };
}


#endif
