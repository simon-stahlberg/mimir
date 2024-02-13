#ifndef MIMIR_SEARCH_SUCCESSOR_STATE_GENERATORS_BITSET_HPP_
#define MIMIR_SEARCH_SUCCESSOR_STATE_GENERATORS_BITSET_HPP_

#include "interface.hpp"

#include <flatmemory/flatmemory.hpp>


namespace mimir
{

/**
 * Implementation class
*/
template<IsPlanningModeTag P>
class SSG<SSGDispatcher<P, BitsetStateTag>>
    : public ISSG<SSG<SSGDispatcher<P, BitsetStateTag>>>
{
private:
    using ConstStateView = ConstView<StateDispatcher<BitsetStateTag, P>>;
    using ConstActionView = ConstView<ActionDispatcher<P, BitsetStateTag>>;

    BitsetStateSet m_states;
    Builder<StateDispatcher<BitsetStateTag, P>> m_state_builder;

    /* Implemenent ISSG interface */
    template<typename>
    friend class ISSG;

    [[nodiscard]] ConstStateView get_or_create_initial_state_impl(Problem problem) {
        // create the state
        int next_state_id = m_states.size();
        m_state_builder.get_id() = next_state_id;
        auto& flatmemory_builder = m_state_builder.get_flatmemory_builder();
        flatmemory_builder.finish();
        return ConstStateView(m_states.insert(flatmemory_builder));
    }

    [[nodiscard]] ConstStateView get_or_create_successor_state_impl(ConstStateView state, ConstActionView action) {
        // create a grounded state.
        // TODO (Dominik): implement
        return ConstStateView(nullptr);
    }
};


}

#endif
