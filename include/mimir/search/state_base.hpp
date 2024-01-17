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
    using Config = typename TypeTraits<Derived>::ConfigType;

    StateBase() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

public:
    // Define common interface for states.
};


/// @brief A concrete state.
template<typename Config>
class State : public StateBase<State<Config>>, IDMixin<State<Config>> {
private:
    // Implement configuration independent functionality.
};


template<typename Config>
struct TypeTraits<State<Config>> {
    using ConfigType = Config;
};

}  // namespace mimir

#endif  // MIMIR_SEARCH_STATE_BASE_HPP_
