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


private:
    [[nodiscard]] State<Grounded> get_or_create_initial_state_impl(Problem problem) {
        // create a grounded state.
        // TODO (Dominik): implement
        return nullptr;
    }

    [[nodiscard]] State<Grounded> get_or_create_successor_state_impl(State<Grounded> state, GroundAction action) {
        // create a grounded state.
        // TODO (Dominik): implement
        return nullptr;
    }

    // Give access to the private interface implementations.
    friend class StateRepositoryBase<StateRepository<Grounded>>;

public:
    StateRepository() { }
};

}  // namespace mimir

#endif  // MIMIR_SEARCH_GROUNDED_STATE_REPOSITORY_HPP_
