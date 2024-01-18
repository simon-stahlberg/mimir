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
class State<Grounded> : public StateBase<State<Grounded>>, public IDMixin<State<Grounded>> {
    // Implement configuration specific functionality.
private:
    State(int index) : IDMixin(index) { }

    friend class StateBase<State<Grounded>>;
    friend class StateRepository<Grounded>;
};

}  // namespace mimir

#endif  // MIMIR_SEARCH_GROUNDED_STATE_HPP_
