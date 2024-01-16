#ifndef MIMIR_SEARCH_STATE_FACTORY_BASE_HPP_
#define MIMIR_SEARCH_STATE_FACTORY_BASE_HPP_

#include "state_base.hpp"

#include "../common/config.hpp"
#include "../common/mixins.hpp"

#include <stdexcept>
#include <vector>

namespace mimir
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

}  // namespace mimir

#endif  // MIMIR_SEARCH_STATE_FACTORY_BASE_HPP_
