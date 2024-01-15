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
class StateBase {
private:
    StateBase() = default;
    friend Derived;

public:
    // Define common interface for states.
};

class GroundedState : public StateBase<GroundedState> {
public:
    // Implement interface
};

class LiftedState : public StateBase<GroundedState> {
public:
    // Implement interface
};


/// @brief CRTP based interface (Mixin) for StateFactory.
/// @tparam Derived
/// @tparam State
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

/// @brief Concrete implementation to construct grounded states.
class GroundedStateFactory : public StateFactoryBase<GroundedStateFactory> {
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
template<typename Derived>
class SearchBase {
private:
    SearchBase() = default;
    friend Derived;

public:
    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }

    void search() {
        self().search_impl();
    }
};


template<typename State>
class BreadthFirstSearch : public SearchBase<BreadthFirstSearch<State>> {
    // The above interface should allow a general implementation of BrFs independent of the type of the state
};


// In cases where we really need specializations, which we should not on high level code.
template<>
class BreadthFirstSearch<GroundedState> : public SearchBase<BreadthFirstSearch<GroundedState>> {
public:
    void search_impl() {
        // implementation
    }
};

}  // namespace formalism

#endif  // MIMIR_FORMALISM_IDEA_HPP_
