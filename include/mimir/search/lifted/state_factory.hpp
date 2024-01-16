#ifndef MIMIR_SEARCH_LIFTED_STATE_FACTORY_HPP_
#define MIMIR_SEARCH_LIFTED_STATE_FACTORY_HPP_

#include "state.hpp"

#include "../config.hpp"
#include "../state_factory_base.hpp"
#include "../../common/mixins.hpp"

#include <stdexcept>
#include <vector>

namespace mimir
{

/// @brief Concrete implementation of a state factory for lifted states.
template<>
class StateFactory<Lifted> : public StateFactoryBase<StateFactory<Lifted>> {
    // Implement configuration specific functionality.
private:
    template<typename... Args>
    State<Lifted> create_impl(Args&& ...args) {
        // create a lifted state.
        return State<Lifted>(std::forward<Args>(args)...);
    }

    // Give access to the private interface implementations.
    friend class StateFactoryBase<StateFactory<Lifted>>;
};

}  // namespace mimir

#endif  // MIMIR_SEARCH_LIFTED_STATE_FACTORY_HPP_
