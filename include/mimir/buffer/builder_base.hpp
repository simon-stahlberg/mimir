
#ifndef MIMIR_BUFFER_BUILDER_BASE_HPP_
#define MIMIR_BUFFER_BUILDER_BASE_HPP_

#include "char_stream.hpp"

#include "../common/mixins.hpp"


namespace mimir::buffer {

template<typename Derived>
class BuilderBase : public UncopyableMixin<BuilderBase<Derived>> {
private:
    BuilderBase() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

    CharStream m_buffer;

public:
    void finish() { self().finish_impl(); }

    const char* get_data() const { return m_buffer.get_data(); }
    int get_size() const { return m_buffer.get_size(); }

    void clear() { m_buffer.clear() }
};


/// @brief User must provide a fully instantiated template.
/// @tparam T
template<typename T>
class Builder : public BuilderBase<Builder<T>> { };

}


#endif
