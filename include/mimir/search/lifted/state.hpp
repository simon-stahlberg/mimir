#ifndef MIMIR_SEARCH_LIFTED_STATE_HPP_
#define MIMIR_SEARCH_LIFTED_STATE_HPP_

#include "../config.hpp"
#include "../state_base.hpp"
#include "../../common/mixins.hpp"


namespace mimir
{
template<typename Config>
class StateRepository;


/// @brief Concrete implementation of a lifted state.
template<>
class State<Lifted> : public StateBase<State<Lifted>>, IDMixin<State<Lifted>> {
    // Implement configuration specific functionality.
private:
    State<Lifted>(int index) : IDMixin(index) { }

    friend class StateBase<State<Lifted>>;
    friend class StateRepository<Lifted>;
};

}  // namespace mimir

#endif  // MIMIR_SEARCH_LIFTED_STATE_HPP_
