#ifndef MIMIR_SEARCH_STATE_BASE_HPP_
#define MIMIR_SEARCH_STATE_BASE_HPP_

#include "config.hpp"
#include "type_traits.hpp"

#include "../buffer/view_base.hpp"
#include "../common/mixins.hpp"


namespace mimir
{

// Common interface, do not add members to keep the view lightweight.
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
    int get_id() const { return self().get_id_impl(); }
};


// ID class for a state
template<Config C>
struct State { };


// View class
template<Config C>
class View<State<C>> : public StateBase<View<State<C>>> {
};


}  // namespace mimir

#endif  // MIMIR_SEARCH_STATE_BASE_HPP_
