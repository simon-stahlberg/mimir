#ifndef MIMIR_SEARCH_SUCCESSOR_STATE_GENERATORS_DEFAULT_GROUNDED_HPP_
#define MIMIR_SEARCH_SUCCESSOR_STATE_GENERATORS_DEFAULT_GROUNDED_HPP_

#include "default.hpp"


namespace mimir
{

/**
 * Implementation class
*/
template<>
class SSG<SSGDispatcher<DefaultSSGTag, GroundedTag, BitsetStateTag, DefaultActionTag>>
    : public ISSG<SSG<SSGDispatcher<DefaultSSGTag, GroundedTag, BitsetStateTag, DefaultActionTag>>>
{
private:
    using StateView = View<StateDispatcher<BitsetStateTag, GroundedTag>>;
    using ActionView = View<ActionDispatcher<DefaultActionTag, GroundedTag, BitsetStateTag>>;

    UnorderedSet<StateDispatcher<S, P>> m_states;
    Builder<StateDispatcher<S, P>> m_state_builder;

    [[nodiscard]] StateView get_or_create_initial_state_impl(Problem problem) {
        m_state_builder.clear();
        int next_state_id = m_states.get_size();
        // TODO: add more functions to state interface to set the bits, etc.
        m_state_builder.set_num_atoms(5);
        m_state_builder.set_id(next_state_id);
        m_state_builder.finish();
        return m_states.insert(m_state_builder);
    }

    [[nodiscard]] StateView get_or_create_successor_state_impl(StateView state, ActionView action) {
        // create a grounded state.
        // TODO (Dominik): implement
        return StateView(nullptr);
    }

    // Give access to the private interface implementations.
    template<typename>
    friend class ISSG;
};


}
#endif
