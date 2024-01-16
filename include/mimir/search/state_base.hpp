#ifndef MIMIR_SEARCH_STATE_BASE_HPP_
#define MIMIR_SEARCH_STATE_BASE_HPP_

#include "../common/config.hpp"
#include "../common/mixins.hpp"

#include <stdexcept>
#include <vector>


namespace mimir
{

/// @brief Top-level CRTP based interface for a State.
/// @tparam Derived
template<typename Derived>
class StateBase {
private:
    StateBase() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

public:
    // Define common interface for states.
};


/// @brief Templatize a concrete state by a configuration.
template<typename Configuration>
class State : public StateBase<State<Configuration>> {
private:
    // Implement configuration independent functionality.
};

}  // namespace mimir

#endif  // MIMIR_SEARCH_STATE_BASE_HPP_
