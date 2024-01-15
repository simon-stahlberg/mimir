#ifndef MIMIR_SEARCH_STATE_FACTORY_HPP_
#define MIMIR_SEARCH_STATE_FACTORY_HPP_

#include "state.hpp"

#include "../common/config.hpp"
#include "../common/mixins.hpp"

#include <stdexcept>
#include <vector>

namespace mimir::search
{

/// @brief Top-level CRTP based interface for a StateFactory.
/// @tparam Derived
template<typename Derived>
class StateFactoryBase {
private:
    StateFactoryBase() = default;
    friend Derived;

public:
    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }

    /// @brief Common interface for state creation.
    ///        Take some arguments and return a state.
    /// @tparam ...Args are the argument types to create a state.
    /// @param ...args are the arguments.
    /// @return
    template<typename... Args>
    auto create(Args&& ...args) {
        return self().create_impl(std::forward<Args>(args)...);
    }
};


template<typename Configuration>
class StateFactory : StateFactoryBase<StateFactory<Configuration>> {
private:
    // Implement configuration independent functionality.
};


/// @brief Concrete implementation of a state factory for grounded states.
template<>
class StateFactory<Grounded> : public StateFactoryBase<StateFactory<Grounded>> {
private:
    // Implement configuration specific functionality.
    template<typename... Args>
    State<Grounded> create_impl(Args&& ...args) {
        // create a grounded state.
        return State<Grounded>(std::forward<Args>(args)...);
    }
};


/// @brief Concrete implementation of a state factory for grounded states.
template<>
class StateFactory<Lifted> : public StateFactoryBase<StateFactory<Lifted>> {
private:
    // Implement configuration specific functionality.
    template<typename... Args>
    State<Lifted> create_impl(Args&& ...args) {
        // create a lifted state.
        return State<Lifted>(std::forward<Args>(args)...);
    }
};

}  // namespace mimir::search

#endif  // MIMIR_SEARCH_STATE_FACTORY_HPP_
