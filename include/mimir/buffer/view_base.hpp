
#ifndef MIMIR_BUFFER_VIEW_BASE_HPP_
#define MIMIR_BUFFER_VIEW_BASE_HPP_

#include "builder_base.hpp"
#include "byte_stream_utils.hpp"

#include "../algorithms/murmurhash3.hpp"

#include <flatbuffers/flatbuffers.h>

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

    // the underlying flatbuffer
    uint8_t* m_data;

protected:
    explicit ViewBase(uint8_t* data) : m_data(data) { }

public:
    /// @brief Compare the representative data.
    [[nodiscard]] bool operator==(const Derived& other) const {
        return self().are_equal_impl();
    }

    /// @brief Hash the representative data.
    [[nodiscard]] size_t hash() const {
        return self().hash_impl();
    }

    /// @brief The first 4 bytes are always reserved for the size, see builder
    [[nodiscard]] uint8_t* get_buffer_pointer() { return m_data; }

    [[nodiscard]] const uint8_t* get_buffer_pointer() const { return m_data; }

    [[nodiscard]] uint32_t get_size() const { return *reinterpret_cast<const flatbuffers::uoffset_t*>(m_data); }
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
