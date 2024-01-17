#ifndef MIMIR_SEARCH_LIFTED_STATE_REPOSITORY_HPP_
#define MIMIR_SEARCH_LIFTED_STATE_REPOSITORY_HPP_

#include "state.hpp"

#include "../config.hpp"
#include "../state_repository_base.hpp"
#include "../../common/mixins.hpp"


namespace mimir
{

/// @brief Concrete implementation of a state factory for lifted states.
template<>
class StateRepository<Lifted> : public StateRepositoryBase<StateRepository<Lifted>> {
    // Implement configuration specific functionality.
private:
    State<Lifted> get_or_create_initial_state_impl(const Problem& problem) {
        // create a lifted state.
        // TODO (Dominik): implement
        return State<Lifted>(0);
    }

    State<Lifted> get_or_create_successor_state_impl(const State<Lifted>& state, const GroundAction& action) {
        // create a lifted state.
        // TODO (Dominik): implement
        return State<Lifted>(0);
    }

    State<Lifted> lookup_state_impl(const ID<State<Lifted>>& state_id) {
        // create a lifted state.
        // TODO (Dominik): implement
        return State<Lifted>(0);
    }

    // Give access to the private interface implementations.
    friend class StateRepositoryBase<StateRepository<Lifted>>;
};

}  // namespace mimir

#endif  // MIMIR_SEARCH_LIFTED_STATE_REPOSITORY_HPP_
