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
class SuccessorStateGenerator<GroundedTag> : public SuccessorStateGeneratorBase<SuccessorStateGenerator<GroundedTag>> {
private:
    [[nodiscard]] View<StateTag<GroundedTag>> get_or_create_initial_state_impl(Problem problem) {
        this->m_state_builder.clear();
        // create the state
        int next_state_id = this->m_states.get_size();
        this->m_state_builder.set_id(next_state_id);
        this->m_state_builder.finish();
        return this->m_states.insert(this->m_state_builder);
    }

    [[nodiscard]] View<StateTag<GroundedTag>> get_or_create_successor_state_impl(View<StateTag<GroundedTag>> state, GroundAction action) {
        // create a grounded state.
        // TODO (Dominik): implement
        return View<StateTag<GroundedTag>>(nullptr);
    }

    // Give access to the private interface implementations.
    friend class SuccessorStateGeneratorBase<SuccessorStateGenerator<GroundedTag>>;
};

}  // namespace mimir

#endif  // MIMIR_SEARCH_GROUNDED_SUCCESSOR_STATE_GENERATOR_HPP_
