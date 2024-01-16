#ifndef MIMIR_SEARCH_STATE_BASE_HPP_
#define MIMIR_SEARCH_STATE_BASE_HPP_

#include "type_traits.hpp"

#include "../common/mixins.hpp"


namespace mimir
{

/// @brief Top-level CRTP based interface for a State.
/// @tparam Derived
template<typename Derived>
class StateBase {
private:
    using Configuration = typename TypeTraits<Derived>::ConfigurationType;

    StateBase() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

public:
    // Define common interface for states.
};


/// @brief A concrete state.
template<typename Configuration>
class State : public StateBase<State<Configuration>> {
private:
    // Implement configuration independent functionality.
};


template<typename Configuration>
struct TypeTraits<State<Configuration>> {
    using ConfigurationType = Configuration;
};

}  // namespace mimir

#endif  // MIMIR_SEARCH_STATE_BASE_HPP_
