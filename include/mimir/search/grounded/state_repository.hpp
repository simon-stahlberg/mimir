#ifndef MIMIR_SEARCH_GROUNDED_STATE_REPOSITORY_HPP_
#define MIMIR_SEARCH_GROUNDED_STATE_REPOSITORY_HPP_

#include "state.hpp"

#include "../config.hpp"
#include "../state_repository_base.hpp"
#include "../../common/mixins.hpp"


namespace mimir
{

/// @brief Concrete implementation of a state factory for grounded states.
template<>
class StateRepository<Grounded> : public StateRepositoryBase<StateRepository<Grounded>> {
    // Implement configuration specific functionality.

    // TODO (Dominik): just temporary to get a reference returned. implement the storage
    State<Grounded> some_state;

private:
    const State<Grounded>& get_or_create_initial_state_impl(const Problem& problem) {
        // create a grounded state.
        // TODO (Dominik): implement
        return some_state;
    }

    const State<Grounded>& get_or_create_successor_state_impl(const State<Grounded>& state, const GroundAction& action) {
        // create a grounded state.
        // TODO (Dominik): implement
        return some_state;
    }

    const State<Grounded>& lookup_state_impl(const ID<State<Grounded>>& state_id) {
        // create a lifted state.
        // TODO (Dominik): implement
        return some_state;
    }

    // Give access to the private interface implementations.
    friend class StateRepositoryBase<StateRepository<Grounded>>;

public:
    StateRepository() : some_state(State<Grounded>(0)) { }
};

}  // namespace mimir

#endif  // MIMIR_SEARCH_GROUNDED_STATE_REPOSITORY_HPP_
