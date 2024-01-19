#ifndef MIMIR_SEARCH_LIFTED_STATE_VIEW_HPP_
#define MIMIR_SEARCH_LIFTED_STATE_VIEW_HPP_

#include "../state_view_base.hpp"
#include "../../common/mixins.hpp"


namespace mimir
{
template<Config C>
class StateRepository;


/// @brief Concrete implementation of a lifted state.
template<>
class View<State<Lifted>> : public StateBase<View<State<Lifted>>> {
    // Implement configuration specific functionality.
private:
    friend class StateBase<View<State<Lifted>>>;
    friend class StateRepository<Lifted>;

public:
    View(char* data, size_t size) : m_data(data), m_size(size) { }
};


}  // namespace mimir

#endif  // MIMIR_SEARCH_LIFTED_STATE_HPP_
