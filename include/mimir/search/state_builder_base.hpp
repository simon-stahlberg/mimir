#ifndef MIMIR_SEARCH_STATE_BUILDER_BASE_HPP_
#define MIMIR_SEARCH_STATE_BUILDER_BASE_HPP_

#include "config.hpp"
#include "type_traits.hpp"

#include "../common/mixins.hpp"

#include <cstdint>


namespace mimir
{

/// TODO (Dominik): This a a case of the buffer pattern, make state a view on the memory layout created by the builder.

/// @brief Top-level CRTP based interface for a StateBuilder.
///        A StateBuilder acts as an intermediate representation of a State
///        with the additional purpose of reusing memory.
/// @tparam Derived
template<typename Derived>
class StateBuilderBase {
private:
    using C = typename TypeTraits<Derived>::ConfigType;

    StateBuilderBase() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

public:
    void set_fact(/* todo */) { self().set_fact_impl(/* todo */); }

    void finish();

    const uint8_t* get_buffer_pointer();
    int get_size() const;
};


/// @brief A concrete state builder.
template<Config C>
class StateBuilder : public StateBuilderBase<StateBuilder<C>> {
private:
    // Implement Config independent functionality.
};


template<Config C>
struct TypeTraits<StateBuilder<C>> {
    using ConfigType = C;
};

}  // namespace mimir

#endif  // MIMIR_SEARCH_STATE_BUILDER_BASE_HPP_
