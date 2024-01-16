#ifndef MIMIR_SEARCH_GROUNDED_STATE_REPOSITORY_HPP_
#define MIMIR_SEARCH_GROUNDED_STATE_REPOSITORY_HPP_

#include "state.hpp"

#include "../config.hpp"
#include "../state_repository_base.hpp"
#include "../../common/mixins.hpp"

#include <stdexcept>
#include <vector>

namespace mimir
{

/// @brief Concrete implementation of a state factory for grounded states.
template<>
class StateRepository<Grounded> : public StateRepositoryBase<StateRepository<Grounded>> {
    // Implement configuration specific functionality.
private:
    template<typename... Args>
    State<Grounded> create_impl(Args&& ...args) {
        // create a grounded state.
        return State<Grounded>(std::forward<Args>(args)...);
    }

    // Give access to the private interface implementations.
    friend class StateRepositoryBase<StateRepository<Grounded>>;
};

}  // namespace mimir

#endif  // MIMIR_SEARCH_GROUNDED_STATE_REPOSITORY_HPP_
