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
    [[nodiscard]] State<Lifted> get_or_create_initial_state_impl(Problem problem) {
        // create a lifted state.
        // TODO (Dominik): implement
        return nullptr;
    }

    [[nodiscard]] State<Lifted> get_or_create_successor_state_impl(State<Lifted> state, GroundAction action) {
        // create a lifted state.
        // TODO (Dominik): implement
        return nullptr;
    }

    // Give access to the private interface implementations.
    friend class StateRepositoryBase<StateRepository<Lifted>>;


public:
    StateRepository() { }
};

}  // namespace mimir

#endif  // MIMIR_SEARCH_LIFTED_STATE_REPOSITORY_HPP_
