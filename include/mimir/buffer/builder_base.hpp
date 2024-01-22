
#ifndef MIMIR_BUFFER_BUILDER_BASE_HPP_
#define MIMIR_BUFFER_BUILDER_BASE_HPP_

#include "byte_stream.hpp"

#include "../common/mixins.hpp"

#include <cstdint>


namespace mimir {

/**
 * Data types.
*/
using data_size_type = uint32_t;


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
    BuilderBase() : m_is_finished(false) { }
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

    ByteStream m_buffer;

    // do not allow data access in unfinished state.
    bool m_is_finished;

protected:
    data_size_type calculate_size() const {
        return sizeof(data_size_type)  // first 4 bytes are reserved for the amount of data.
             + self().calculate_size_impl();  // the remaining bytes of Derived
    }

public:
    /// @brief Write the data to the buffer.
    void finish() {
        // write the base data.
        data_size_type size = this->calculate_size();
        this->m_buffer.write(size);
        // write the derived data.
        self().finish_impl();
        m_is_finished = true;
    }

    /// @brief Retrieve the buffer.
    ///        The user can copy it to the final location.
    [[nodiscard]] ByteStream& get_buffer() {
        if (!m_is_finished) {
            throw std::runtime_error("Attempted to retrieve buffer of unfinished builder. Call finish() first.");
        }
        return m_buffer;
    }

    /// @brief Retrieve the buffer.
    ///        The user can copy it to the final location.
    [[nodiscard]] const ByteStream& get_buffer() const {
        if (!m_is_finished) {
            throw std::runtime_error("Attempted to retrieve buffer of unfinished builder. Call finish() first.");
        }
        return m_buffer;
    }

    /// @brief Clear the builder for reuse.
    void clear() {
        m_buffer.clear();
        m_is_finished = false;
    }
};


/**
 * Implementation class.
 *
 * Provide an implementation for T by providing fully specified template.
*/
template<typename Tag>
class Builder : public BuilderBase<Builder<Tag>> {};

}


#endif
