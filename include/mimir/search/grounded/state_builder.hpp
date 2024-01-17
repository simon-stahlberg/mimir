#ifndef MIMIR_SEARCH_GROUNDED_STATE_BUILDER_HPP_
#define MIMIR_SEARCH_GROUNDED_STATE_BUILDER_HPP_

#include "../config.hpp"
#include "../state_builder_base.hpp"
#include "../../common/mixins.hpp"


namespace mimir
{

/// @brief Concrete implementation of a grounded state.
template<>
class StateBuilder<Grounded> : public StateBuilderBase<StateBuilder<Grounded>> {
    // Implement configuration specific functionality.
private:

    friend class StateBuilderBase<StateBuilder<Grounded>>;
};

}  // namespace mimir

#endif  // MIMIR_SEARCH_GROUNDED_STATE_BUILDER_HPP_
