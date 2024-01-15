#ifndef MIMIR_SEARCH_STATE_HPP_
#define MIMIR_SEARCH_STATE_HPP_

#include "../common/config.hpp"
#include "../common/mixins.hpp"

#include <stdexcept>
#include <vector>


namespace mimir::search
{

/// @brief Top-level CRTP based interface for a State.
/// @tparam Derived
template<typename Derived>
class StateBase {
private:
    StateBase() = default;
    friend Derived;

public:
    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }

    // Define common interface for states.
};


/// @brief Templatize a concrete state by a configuration.
template<typename Configuration>
class State : public StateBase<State<Configuration>> {
private:
    // Implement configuration independent functionality.
};


/// @brief Concrete implementation of a grounded state.
template<>
class State<Grounded> : public StateBase<State<Grounded>> {
private:
    // Implement configuration specific functionality.
};


/// @brief Concrete implementation of a lifted state.
template<>
class State<Lifted> : public StateBase<State<Lifted>> {
private:
    // Implement configuration specific functionality.
};

}  // namespace mimir::search

#endif  // MIMIR_SEARCH_STATE_HPP_
