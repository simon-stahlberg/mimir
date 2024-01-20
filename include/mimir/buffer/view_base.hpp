
#ifndef MIMIR_BUFFER_VIEW_BASE_HPP_
#define MIMIR_BUFFER_VIEW_BASE_HPP_

#include "char_stream_utils.hpp"
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

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

    // Basic meta data that every view must contain to be equality comparable and hashable
    char* m_data;

protected:
    explicit ViewBase(char* data) : m_data(data) { }

    /// @brief Access the data to be interpreted by derived classes.
    [[nodiscard]] char* get_data() { return m_data; }
    [[nodiscard]] const char* get_data() const { return m_data; }
    /// @brief The first 4 bytes are always reserved for the size.
    [[nodiscard]] DataSizeType get_size() const { return read_value<DataSizeType>(m_data); }

public:
    [[nodiscard]] bool operator==(const Derived& other) const {
        return self().operator_equal_impl(other);
    }

    [[nodiscard]] size_t hash() const {
        return self().hash_impl();
    }
};


/**
 * Implementation class for some type T.
 * Provide an implementation for T by providing fully specified template.
*/
template<typename T>
class View : public ViewBase<View<T>> {};

}


#endif
