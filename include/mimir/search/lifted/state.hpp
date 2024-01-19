#ifndef MIMIR_SEARCH_LIFTED_STATE_HPP_
#define MIMIR_SEARCH_LIFTED_STATE_HPP_

#include "../state_base.hpp"
#include "../../common/mixins.hpp"


namespace mimir
{
template<Config C>
class StateRepository;


/// @brief Concrete implementation of a lifted state.
template<>
class StateImpl<Lifted> : public StateBase<StateImpl<Lifted>> {
    // Implement configuration specific functionality.
private:
    friend class StateBase<StateImpl<Lifted>>;
    friend class StateRepository<Lifted>;
};


}  // namespace mimir

#endif  // MIMIR_SEARCH_LIFTED_STATE_HPP_
