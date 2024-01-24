#ifndef MIMIR_SEARCH_SUCCESSOR_STATE_GENERATORS_DEFAULT_LIFTED_HPP_
#define MIMIR_SEARCH_SUCCESSOR_STATE_GENERATORS_DEFAULT_LIFTED_HPP_

#include "default.hpp"


namespace mimir
{

template<>
class SSG<WrappedSSGTag<DefaultSSGTag, LiftedTag, BitsetStateTag, DefaultActionTag>>
    : public SSGBase<SSG<WrappedSSGTag<DefaultSSGTag, LiftedTag, BitsetStateTag, DefaultActionTag>>> {
private:
    using StateView = View<WrappedStateTag<BitsetStateTag, LiftedTag>>;
    using ActionView = View<WrappedActionTag<DefaultActionTag, LiftedTag, BitsetStateTag>>;

    [[nodiscard]] StateView get_or_create_initial_state_impl(Problem problem) {
        this->m_state_builder.clear();
        // create the state
        int next_state_id = this->m_states.get_size();
        this->m_state_builder.set_id(next_state_id);
        this->m_state_builder.finish();
        return this->m_states.insert(this->m_state_builder);
    }

    [[nodiscard]] StateView get_or_create_successor_state_impl(StateView state, ActionView action) {
        // create a grounded state.
        // TODO (Dominik): implement
        return StateView(nullptr);
    }

    // Give access to the private interface implementations.
    template<typename>
    friend class SSGBase;
};


}  // namespace mimir

#endif  // MIMIR_SEARCH_SUCCESSOR_STATE_GENERATORS_DEFAULT_LIFTED_HPP_
