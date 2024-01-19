#ifndef MIMIR_SEARCH_GROUNDED_STATE_HPP_
#define MIMIR_SEARCH_GROUNDED_STATE_HPP_

#include "../state_base.hpp"
#include "../../common/mixins.hpp"


namespace mimir
{
template<Config C>
class StateRepository;


/// @brief Concrete implementation of a grounded state.
template<>
class StateImpl<Grounded> : public StateBase<StateImpl<Grounded>> {
    // Implement configuration specific functionality.
private:
    friend class StateBase<StateImpl<Grounded>>;
    friend class StateRepository<Grounded>;
};

}  // namespace mimir

#endif  // MIMIR_SEARCH_GROUNDED_STATE_HPP_
