#ifndef MIMIR_SEARCH_SUCCESSOR_STATE_GENERATORS_DEFAULT_GROUNDED_HPP_
#define MIMIR_SEARCH_SUCCESSOR_STATE_GENERATORS_DEFAULT_GROUNDED_HPP_

#include "../default.hpp"


namespace mimir
{

/**
 * No ID class since we have all tags.
*/


/**
 * Implementation class
*/
template<>
class SuccessorStateGenerator<DefaultSuccessorStateGeneratorTag<Grounded>> : public SuccessorStateGeneratorBase<SuccessorStateGenerator<DefaultSuccessorStateGeneratorTag<Grounded>>> {
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
    friend class SuccessorStateGeneratorBase<SuccessorStateGenerator<DefaultSuccessorStateGeneratorTag<Grounded>>>;
};

}  // namespace mimir

#endif  // MIMIR_SEARCH_SUCCESSOR_STATE_GENERATORS_DEFAULT_GROUNDED_HPP_
