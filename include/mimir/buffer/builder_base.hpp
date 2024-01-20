
#ifndef MIMIR_BUFFER_BUILDER_BASE_HPP_
#define MIMIR_BUFFER_BUILDER_BASE_HPP_

#include "types.hpp"
#include "char_stream.hpp"

#include "../common/mixins.hpp"


namespace mimir {

/**
 * Interface class
*/
template<typename Derived>
class BuilderBase {
private:
    BuilderBase() : m_is_finished(false) { }
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

    CharStream m_buffer;

    // do not allow data access in unfinished state.
    bool m_is_finished;

protected:
    DataSizeType calculate_size() const {
        return sizeof(DataSizeType)  // first 4 bytes are reserved for the amount of data.
             + self().calculate_size_impl();  // the remaining bytes of Derived
    }

public:
    /// @brief Write the data to the buffer.
    void finish() {
        // write the amount of data to be written first.
        DataSizeType size = this->calculate_size();
        // write the derived data.
        this->m_buffer.write(size);
        self().finish_impl();
        m_is_finished = true;
    }

    /// @brief Retrieve the buffer.
    ///        The user can copy it to the final location.
    [[nodiscard]] const CharStream& get_buffer() const {
        if (!m_is_finished) {
            throw std::runtime_error("Accessing buffer of unfinished building process is not allowed. Call finish() first.");
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
 * Implementation class for some type T.
 * Provide an implementation for T by providing fully specified template.
*/
template<typename T>
class Builder : public BuilderBase<Builder<T>> {};

}


#endif
