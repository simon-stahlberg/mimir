#ifndef MIMIR_SEARCH_LIFTED_SUCCESSOR_STATE_GENERATOR_HPP_
#define MIMIR_SEARCH_LIFTED_SUCCESSOR_STATE_GENERATOR_HPP_

#include "../state.hpp"
#include "state_view.hpp"
#include "state_builder.hpp"

#include "../config.hpp"
#include "../successor_state_generator.hpp"


namespace mimir
{

/// @brief Concrete implementation of a state factory for lifted states.
template<>
class SuccessorStateGenerator<Lifted> : public SuccessorStateGeneratorBase<SuccessorStateGenerator<Lifted>> {
private:
    [[nodiscard]] View<State<Lifted>> get_or_create_initial_state_impl(Problem problem) {
        this->m_state_builder.clear();
        // create the state
        int next_state_id = this->m_states.get_size();
        this->m_state_builder.set_id(next_state_id);
        this->m_state_builder.finish();
        return this->m_states.insert(this->m_state_builder);
    }

    [[nodiscard]] View<State<Lifted>> get_or_create_successor_state_impl(View<State<Lifted>> state, GroundAction action) {
        // create a lifted state.
        // TODO (Dominik): implement
        return View<State<Lifted>>(nullptr);
    }

    // Give access to the private interface implementations.
    friend class SuccessorStateGeneratorBase<SuccessorStateGenerator<Lifted>>;
};

}  // namespace mimir

#endif  // MIMIR_SEARCH_LIFTED_SUCCESSOR_STATE_GENERATOR_HPP_
