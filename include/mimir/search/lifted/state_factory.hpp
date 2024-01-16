#ifndef MIMIR_SEARCH_LIFTED_STATE_FACTORY_HPP_
#define MIMIR_SEARCH_LIFTED_STATE_FACTORY_HPP_

#include "../state_factory_base.hpp"

#include "state.hpp"

#include "../../common/config.hpp"
#include "../../common/mixins.hpp"

#include <stdexcept>
#include <vector>

namespace mimir
{

/// @brief Concrete implementation of a state factory for lifted states.
template<>
class StateFactory<Lifted> : public StateFactoryBase<StateFactory<Lifted>> {
private:
    // Implement configuration specific functionality.
    template<typename... Args>
    State<Lifted> create_impl(Args&& ...args) {
        // create a lifted state.
        return State<Lifted>(std::forward<Args>(args)...);
    }
};

}  // namespace mimir

#endif  // MIMIR_SEARCH_LIFTED_STATE_FACTORY_HPP_
