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
    State<Lifted> create_impl(const StateBuilder<Lifted>& builder) {
        // create a lifted state.
        return State<Lifted>();
    }

    // Give access to the private interface implementations.
    friend class StateRepositoryBase<StateRepository<Lifted>>;
};

}  // namespace mimir

#endif  // MIMIR_SEARCH_LIFTED_STATE_REPOSITORY_HPP_
