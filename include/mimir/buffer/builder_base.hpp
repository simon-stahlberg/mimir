
#ifndef MIMIR_BUFFER_BUILDER_BASE_HPP_
#define MIMIR_BUFFER_BUILDER_BASE_HPP_

#include "../common/mixins.hpp"

#include <flatbuffers/flatbuffers.h>

#include <cstdint>
#include <iostream>
#include <iomanip>

namespace mimir {


/**
 * Interface class
 *
 * Our concrete builders use flatbuffers with a uint32_t prefix that encodes the size of the buffer
 *  __________________________________
 * |                 |               |
 * | size (uint32_t) | <actual data> |
 * |_________________|_______________|
*/
template<typename Derived>
class BuilderBase {
private:
    flatbuffers::FlatBufferBuilder m_flatbuffers_builder;
    typename TypeTraits<Derived>::TypeFlatBuilder m_type_builder;

    BuilderBase()
        : m_flatbuffers_builder(1024)
        , m_type_builder(m_flatbuffers_builder) { }
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

public:
    /// @brief Write the data to the buffer.
    void finish() { m_flatbuffers_builder.FinishSizePrefixed(m_type_builder.Finish()); }

    [[nodiscard]] uint8_t* get_buffer_pointer() { return m_flatbuffers_builder.GetBufferPointer(); }

    [[nodiscard]] const uint8_t* get_buffer_pointer() const { return m_flatbuffers_builder.GetBufferPointer(); }

    // According to flatbuffers api: "Finish a buffer with a 32 bit size field pre-fixed (size of the buffer following the size field)." Hence we must add the size of the prefix.
    [[nodiscard]] uint32_t get_size() const { return *reinterpret_cast<const flatbuffers::uoffset_t*>(get_buffer_pointer()) + sizeof(flatbuffers::uoffset_t); }

    /// @brief Clear the builder for reuse.
    void clear() { m_flatbuffers_builder.Clear(); }
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
