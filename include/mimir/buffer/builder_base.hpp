
#ifndef MIMIR_BUFFER_BUILDER_BASE_HPP_
#define MIMIR_BUFFER_BUILDER_BASE_HPP_

#include "buffer.hpp"

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

    BinaryBuffer m_buffer;

public:
    void finish() { self().finish_impl(); }

    const char* get_data() const { return m_buffer.get_data(); }
    int get_size() const { return m_buffer.get_size(); }

    void clear() { m_buffer.clear() }
};

}


#endif
