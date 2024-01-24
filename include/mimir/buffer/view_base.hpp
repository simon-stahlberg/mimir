
#ifndef MIMIR_BUFFER_VIEW_BASE_HPP_
#define MIMIR_BUFFER_VIEW_BASE_HPP_

#include "builder_base.hpp"
#include "byte_stream_utils.hpp"

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

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

    // Basic meta data that every view must contain to be equality comparable and hashable
    char* m_data;

    [[nodiscard]] size_t get_offset_to_representative_data() const { return self().get_offset_to_representative_data_impl(); }

protected:
    explicit ViewBase(char* data) : m_data(data) { }

    /// @brief Access the data to be interpreted by derived classes.
    [[nodiscard]] char* get_data() { return m_data; }
    [[nodiscard]] const char* get_data() const { return m_data; }

public:
    /// @brief Compare the representative data.
    [[nodiscard]] bool operator==(const Derived& other) const {
        if (get_size() != other.get_size()) return false;
        // Compare representative data.
        size_t offset = get_offset_to_representative_data();
        return (std::memcmp(get_data() + offset, other.get_data() + offset, get_size() - offset) == 0);
    }

    /// @brief Hash the representative data.
    [[nodiscard]] size_t hash() const {
        size_t seed = get_size();
        int64_t hash[2];
        size_t offset = get_offset_to_representative_data();
        MurmurHash3_x64_128(get_data() + offset, get_size() - offset, seed, hash);
        return static_cast<std::size_t>(hash[0] + 0x9e3779b9 + (hash[1] << 6) + (hash[1] >> 2));
    }

    /// @brief The first 4 bytes are always reserved for the size, see builder
    [[nodiscard]] data_size_type get_size() const { return read_value<data_size_type>(m_data); }
};


/**
 * Implementation class.
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
            return view.hash();
        }
    };
}  // namespace std


#endif
