
#ifndef MIMIR_BUFFER_BUILDER_BASE_HPP_
#define MIMIR_BUFFER_BUILDER_BASE_HPP_

#include "../common/mixins.hpp"

#include <flatbuffers/flatbuffers.h>

#include <cstdint>


namespace mimir
{

/**
 * Interface class
 *
 * Our concrete builders usually use flatbuffers with a uint32_t prefix that encodes the size of the buffer
 *  __________________________________
 * |                 |               |
 * | size (uint32_t) | <actual data> |
 * |_________________|_______________|
*/
template<typename Derived>
class IBuilderBase
{
private:
    IBuilderBase() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

public:
    /// @brief Write the data to the buffer.
    void finish() { self().finish_impl(); }

    /// @brief Clear the builder for reuse.
    void clear() { self().clear_impl(); }

    // TODO: get rid of the non const version.
    [[nodiscard]] uint8_t* get_buffer_pointer() { return self().get_buffer_pointer_impl(); }
    [[nodiscard]] const uint8_t* get_buffer_pointer() const { return self().get_buffer_pointer_impl(); }
    [[nodiscard]] uint32_t get_size() const { return self().get_size_impl(); }
};


/**
 * General implementation class.
 *
 * Specialize the wrapped tag to provide your own implementation of a successor state generator.
*/
template<typename T>
class Builder : public IBuilderBase<Builder<T>> {};

}


#endif
