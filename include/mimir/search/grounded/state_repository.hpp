#ifndef MIMIR_SEARCH_GROUNDED_STATE_REPOSITORY_HPP_
#define MIMIR_SEARCH_GROUNDED_STATE_REPOSITORY_HPP_

#include "state_view.hpp"
#include "state_builder.hpp"

#include "../config.hpp"
#include "../state_repository.hpp"


namespace mimir
{

/// @brief Concrete implementation of a state factory for grounded states.
template<>
class StateRepository<Grounded> : public StateRepositoryBase<StateRepository<Grounded>> {
private:
    [[nodiscard]] View<State<Grounded>> get_or_create_initial_state_impl(Problem problem) {
        this->m_state_builder.clear();
        // create the state
        int next_state_id = this->m_states.get_size();
        this->m_state_builder.set_id(next_state_id);
        this->m_state_builder.finish();
        return this->m_states.insert(this->m_state_builder);
    }

    [[nodiscard]] View<State<Grounded>> get_or_create_successor_state_impl(View<State<Grounded>> state, GroundAction action) {
        // create a grounded state.
        // TODO (Dominik): implement
        return View<State<Grounded>>(nullptr);
    }

    // Give access to the private interface implementations.
    friend class StateRepositoryBase<StateRepository<Grounded>>;
};

}  // namespace mimir

#endif  // MIMIR_SEARCH_GROUNDED_STATE_REPOSITORY_HPP_
