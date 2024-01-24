#ifndef MIMIR_SEARCH_SUCCESSOR_STATE_GENERATORS_DEFAULT_IMPL_HPP_
#define MIMIR_SEARCH_SUCCESSOR_STATE_GENERATORS_DEFAULT_IMPL_HPP_

#include "default.hpp"

/*
 * Table of Contents:
 * - Grounded Implementation
 * - Lifted Implementation
 */

namespace mimir
{

/**
 * Implementation class
*/
template<>
class SuccessorStateGenerator<WrappedSuccessorStateGeneratorTag<DefaultSuccessorStateGeneratorTag, GroundedTag>>
    : public SuccessorStateGeneratorBase<SuccessorStateGenerator<WrappedSuccessorStateGeneratorTag<DefaultSuccessorStateGeneratorTag, GroundedTag>>> {
private:
    [[nodiscard]] View<State<GroundedTag>> get_or_create_initial_state_impl(Problem problem) {
        this->m_state_builder.clear();
        // create the state
        int next_state_id = this->m_states.get_size();
        this->m_state_builder.set_id(next_state_id);
        this->m_state_builder.finish();
        return this->m_states.insert(this->m_state_builder);
    }

    [[nodiscard]] View<State<GroundedTag>> get_or_create_successor_state_impl(View<State<GroundedTag>> state, GroundAction action) {
        // create a grounded state.
        // TODO (Dominik): implement
        return View<State<GroundedTag>>(nullptr);
    }

    // Give access to the private interface implementations.
    template<typename>
    friend class SuccessorStateGeneratorBase;
};


template<>
class SuccessorStateGenerator<WrappedSuccessorStateGeneratorTag<DefaultSuccessorStateGeneratorTag, LiftedTag>>
    : public SuccessorStateGeneratorBase<SuccessorStateGenerator<WrappedSuccessorStateGeneratorTag<DefaultSuccessorStateGeneratorTag, LiftedTag>>> {
private:
    [[nodiscard]] View<State<LiftedTag>> get_or_create_initial_state_impl(Problem problem) {
        this->m_state_builder.clear();
        // create the state
        int next_state_id = this->m_states.get_size();
        this->m_state_builder.set_id(next_state_id);
        this->m_state_builder.finish();
        return this->m_states.insert(this->m_state_builder);
    }

    [[nodiscard]] View<State<LiftedTag>> get_or_create_successor_state_impl(View<State<LiftedTag>> state, GroundAction action) {
        // create a grounded state.
        // TODO (Dominik): implement
        return View<State<LiftedTag>>(nullptr);
    }

    // Give access to the private interface implementations.
    template<typename>
    friend class SuccessorStateGeneratorBase;
};


}  // namespace mimir

#endif  // MIMIR_SEARCH_SUCCESSOR_STATE_GENERATORS_DEFAULT_GROUNDED_HPP_
