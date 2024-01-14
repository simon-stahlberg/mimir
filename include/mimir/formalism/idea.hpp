#ifndef MIMIR_FORMALISM_IDEA_HPP_
#define MIMIR_FORMALISM_IDEA_HPP_

#include "../common/mixins.hpp"

#include <stdexcept>
#include <vector>

namespace mimir::formalism
{

/// @brief CRTP based interface (Mixin) for State.
/// @tparam Derived
template<typename Derived>
class StateMixin {
private:
    StateMixin() = default;
    friend Derived;

public:
    // Define common interface for states.
};

class GroundedState : public StateMixin<GroundedState> {
public:
    // Implement interface
};

class LiftedState : public StateMixin<GroundedState> {
public:
    // Implement interface
};


/// @brief CRTP based interface (Mixin) for StateFactory.
/// @tparam Derived
/// @tparam State
template<typename Derived, typename State>
class StateFactoryMixin {
private:
    StateFactoryMixin() = default;
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
    State create(Args&& ...args) {
        return self().create_impl(std::forward<Args>(args)...);
    }
};

/// @brief Concrete implementation to construct grounded states.
class GroundedStateFactory : public StateFactoryMixin<GroundedStateFactory, GroundedState> {
public:
    template<typename... Args>
    GroundedState create_impl(Args&& ...args) {
        // create a grounded state.
        return GroundedState(std::forward<Args>(args)...);
    }
};


/* Example search algorithm where the common State interface is insufficient.
   Again, we simply use CRTP based interface (Mixin). */
/// @brief CRTP based interface (Mixin) for a BrFS search algorithm
template<typename Derived, typename State>
class BreadthFirstSearchMixin {
private:
    BreadthFirstSearch() = default;
    friend Derived;

public:
    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }

    void search() {
        self().search_impl();
    }
};


class GroundedBreadthFirstSearch : public BreadthFirstSearchMixin<GroundedBreadthFirstSearch, GroundedState> {
public:
    void search_impl() {
        // implementation
    }
};

}  // namespace formalism

#endif  // MIMIR_FORMALISM_IDEA_HPP_
