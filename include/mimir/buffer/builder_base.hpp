
#ifndef MIMIR_BUFFER_BUILDER_BASE_HPP_
#define MIMIR_BUFFER_BUILDER_BASE_HPP_

#include "../common/mixins.hpp"

#include <flatbuffers/flatbuffers.h>

#include <cstdint>


namespace mimir {


/**
 * Interface class
 *
 * The builder base creates the following base memory layout:
 *  ____________________________________________
 * |                |                           |
 * | data_size_type | <Data of derived builder> |
 * |________________|___________________________|
*/
template<typename Derived>
class BuilderBase {
private:
    BuilderBase() { }
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

public:
    /// @brief Write the data to the buffer.
    void finish() { self().finish_impl(); }

    [[nodiscard]] uint8_t* get_buffer_pointer() { return self().get_buffer_pointer_impl(); }

    [[nodiscard]] const uint8_t* get_buffer_pointer() const { return self().get_buffer_pointer_impl(); }

    [[nodiscard]] uint32_t get_size() const { return *reinterpret_cast<const flatbuffers::uoffset_t*>(get_buffer_pointer()); }

    /// @brief Clear the builder for reuse.
    void clear() { self().clear_impl(); }
};


/**
 * Implementation class.
 *
 * Provide an implementation for T by providing fully specified template.
*/
template<typename T>
class Builder : public BuilderBase<Builder<T>> {};

}


#endif
