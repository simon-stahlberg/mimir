#ifndef MIMIR_SEARCH_GROUNDED_STATE_VIEW_HPP_
#define MIMIR_SEARCH_GROUNDED_STATE_VIEW_HPP_

#include "../state_view_base.hpp"
#include "../../common/mixins.hpp"


namespace mimir
{
template<Config C>
class StateRepository;


/// @brief Concrete implementation of a grounded state.
template<>
class View<State<Grounded>> : public StateBase<View<State<Grounded>>> {
    // Implement configuration specific functionality.
private:
    friend class StateBase<View<State<Grounded>>>;
    friend class StateRepository<Grounded>;
};

}  // namespace mimir

#endif  // MIMIR_SEARCH_GROUNDED_STATE_HPP_
