
#ifndef MIMIR_BUFFER_CONTAINERS_VECTOR_HPP_
#define MIMIR_BUFFER_CONTAINERS_VECTOR_HPP_

#include "../builder_base.hpp"
#include "../byte_stream_segmented.hpp"
#include "../view_base.hpp"

#include <cassert>
#include <vector>


namespace mimir {

/// @brief Vector handle different sized objects but only allows push_back and access, no resize.
/// @tparam T
template<typename T>
class Vector {
private:
    // Persistent storage
    ByteStreamSegmented<100000> m_storage;

    // Data to be accessed
    std::vector<View<T>> m_data;

public:
    Vector() = default;
    // Move only
    Vector(const Vector& other) = delete;
    Vector& operator=(const Vector& other) = delete;
    Vector(Vector&& other) = default;
    Vector& operator=(Vector&& other) = default;

    void push_back(const Builder<T>& builder) {
        m_data.push_back(View<T>(m_storage.write(builder.get_data(), builder.get_size()), builder.get_size()));
    }

    [[nodiscard]] View<T> back() {
        return m_data.back();
    }

    [[nodiscard]] const View<T> back() const {
        return m_data.back();
    }

    [[nodiscard]] View<T> operator[](size_t pos) {
        assert(pos <= static_cast<int>(get_size()));
        return m_data[pos];
    }

    [[nodiscard]] const View<T> operator[](size_t pos) const {
        assert(pos <= static_cast<int>(get_size()));
        return m_data[pos];
    }

    [[nodiscard]] size_t get_size() const {
        return m_data.size();
    }
};


/// @brief AutomaticVector handle handles only equally sized objects but allows for resize.
///        AUtomaticVector is sometimes more convenient than Vector, e.g., SearchNodes.
/// @tparam T
template<typename T>
class AutomaticVector {
private:
    // Persistent storage
    ByteStreamSegmented<100000> m_storage;

    // Data to be accessed
    std::vector<View<T>> m_data;

    const Builder<T> m_default_builder;

public:
    AutomaticVector(Builder<T>&& default_builder) : m_default_builder(std::move(default_builder)) { }
    // Move only
    AutomaticVector(const AutomaticVector& other) = delete;
    AutomaticVector& operator=(const AutomaticVector& other) = delete;
    AutomaticVector(AutomaticVector&& other) = default;
    AutomaticVector& operator=(AutomaticVector&& other) = default;

    void push_back(const Builder<T>& builder) {
        m_data.push_back(View<T>(m_storage.write(builder.get_data(), builder.get_size()), builder.get_size()));
    }

    [[nodiscard]] View<T> back() {
        return m_data.back();
    }

    [[nodiscard]] const View<T> back() const {
        return m_data.back();
    }

    [[nodiscard]] View<T> operator[](size_t pos) {
        if (pos >= get_size()) {
            resize(pos);
        }
        return m_data[pos];
    }

    [[nodiscard]] const View<T> operator[](size_t pos) const {
        if (pos >= get_size()) {
            resize(pos);
        }
        return m_data[pos];
    }

    void resize(size_t size) {
        const char* default_data = m_default_builder.get_buffer().get_data();
        size_t default_size = m_default_builder.get_buffer().get_size();
        while (get_size() <= size) {
            char* written_data = m_storage.write(default_data, default_size);
            m_data.push_back(View<T>(written_data));
        }
    }

    [[nodiscard]] size_t get_size() const {
        return m_data.size();
    }
};


}

#endif
