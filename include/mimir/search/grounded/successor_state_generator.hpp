#ifndef MIMIR_SEARCH_GROUNDED_SUCCESSOR_STATE_GENERATOR_HPP_
#define MIMIR_SEARCH_GROUNDED_SUCCESSOR_STATE_GENERATOR_HPP_

#include "../successor_state_generator.hpp"


namespace mimir
{

/**
 * No ID class since we have all tags.
*/


/**
 * Implementation class
*/
template<>
class SuccessorStateGenerator<Grounded> : public SuccessorStateGeneratorBase<SuccessorStateGenerator<Grounded>> {
private:
    [[nodiscard]] StateView<Grounded> get_or_create_initial_state_impl(Problem problem) {
        this->m_state_builder.clear();
        // create the state
        int next_state_id = this->m_states.get_size();
        this->m_state_builder.set_id(next_state_id);
        this->m_state_builder.finish();
        return this->m_states.insert(this->m_state_builder);
    }

    [[nodiscard]] StateView<Grounded> get_or_create_successor_state_impl(StateView<Grounded> state, GroundAction action) {
        // create a grounded state.
        // TODO (Dominik): implement
        return StateView<Grounded>(nullptr);
    }

    // Give access to the private interface implementations.
    friend class SuccessorStateGeneratorBase<SuccessorStateGenerator<Grounded>>;
};

}  // namespace mimir

#endif  // MIMIR_SEARCH_GROUNDED_SUCCESSOR_STATE_GENERATOR_HPP_
