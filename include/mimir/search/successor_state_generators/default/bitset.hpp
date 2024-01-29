#ifndef MIMIR_SEARCH_SUCCESSOR_STATE_GENERATORS_DEFAULT_BITSET_HPP_
#define MIMIR_SEARCH_SUCCESSOR_STATE_GENERATORS_DEFAULT_BITSET_HPP_

#include "interface.hpp"


namespace mimir
{

/**
 * Implementation class
*/
template<IsPlanningModeTag P>
class SSG<SSGDispatcher<DefaultSSGTag, P, BitsetStateTag>>
    : public ISSG<SSG<SSGDispatcher<DefaultSSGTag, P, BitsetStateTag>>>
{
private:
    using StateView = View<StateDispatcher<BitsetStateTag, P>>;
    using ActionView = View<ActionDispatcher<P, BitsetStateTag>>;

    UnorderedSet<StateDispatcher<BitsetStateTag, P>> m_states;
    Builder<StateDispatcher<BitsetStateTag, P>> m_state_builder;

    [[nodiscard]] StateView get_or_create_initial_state_impl(Problem problem) {
        m_state_builder.clear();
        // create the state
        int next_state_id = m_states.get_size();
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
