#ifndef MIMIR_SEARCH_GROUNDED_STATE_FACTORY_HPP_
#define MIMIR_SEARCH_GROUNDED_STATE_FACTORY_HPP_

#include "../state_factory_base.hpp"

#include "state.hpp"

#include "../../common/config.hpp"
#include "../../common/mixins.hpp"

#include <stdexcept>
#include <vector>

namespace mimir
{

/// @brief Concrete implementation of a state factory for grounded states.
template<>
class StateFactory<Grounded> : public StateFactoryBase<StateFactory<Grounded>> {
    // Implement configuration specific functionality.
private:
    template<typename... Args>
    State<Grounded> create_impl(Args&& ...args) {
        // create a grounded state.
        return State<Grounded>(std::forward<Args>(args)...);
    }

    // Give access to the private interface implementations.
    friend class StateFactoryBase<StateFactory<Grounded>>;
};

}  // namespace mimir

#endif  // MIMIR_SEARCH_GROUNDED_STATE_FACTORY_HPP_
