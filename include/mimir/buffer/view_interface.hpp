
#ifndef MIMIR_BUFFER_VIEW_BASE_HPP_
#define MIMIR_BUFFER_VIEW_BASE_HPP_

#include "builder_interface.hpp"
#include "byte_stream_utils.hpp"

#include "../algorithms/murmurhash3.hpp"

#include <flatbuffers/flatbuffers.h>


namespace mimir {

/**
 * Interface class
 * __________________________________
 * |                 |               |
 * | size (uint32_t) | <actual data> |
 * |_________________|_______________|
*/
template<typename Derived>
class IView
{
private:
    IView() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

public:
    /// @brief Compare the representative data.
    [[nodiscard]] bool operator==(const Derived& other) const { return self().are_equal_impl(other); }

    /// @brief Hash the representative data.
    [[nodiscard]] size_t hash() const { return self().hash_impl(); }

    [[nodiscard]] const uint8_t* get_buffer_pointer() const { return self().get_buffer_pointer_impl(); }

    [[nodiscard]] uint32_t get_size() const { return self().get_size_impl(); }
};


/**
 * General implementation class.
 *
 * Specialize the wrapped tag to provide your own implementation of a successor state generator.
*/
template<typename T>
class View : public IView<View<T>> {};

}


namespace std {
    template<typename T>
    struct hash<mimir::View<T>>
    {
        std::size_t operator()(const mimir::View<T>& view) const {
            return view.hash();
        }
    };
}

#endif
