#ifndef MIMIR_SEARCH_ALGORITHMS_EVENT_HANDLERS_DEBUG_HPP_
#define MIMIR_SEARCH_ALGORITHMS_EVENT_HANDLERS_DEBUG_HPP_

#include "mimir/search/algorithms/event_handlers/interface.hpp"

#include <iostream>

namespace mimir
{

/**
 * Implementation class
 */
class DebugAlgorithmEventHandler : public AlgorithmEventHandlerBase<DebugAlgorithmEventHandler>
{
private:
    /* Implement AlgorithmEventHandlerBase interface */
    friend class AlgorithmEventHandlerBase<DebugAlgorithmEventHandler>;

    void on_generate_state_impl(GroundAction action, State successor_state, const PDDLFactories& pddl_factories) const;

    void on_finish_g_layer_impl(uint64_t g_value, uint64_t num_states) const;

    void on_expand_state_impl(State state, const PDDLFactories& pddl_factories) const;

    void on_start_search_impl(State initial_state, const PDDLFactories& pddl_factories) const;

    void on_end_search_impl() const;

    void on_solved_impl(const GroundActionList& ground_action_plan) const;

    void on_exhausted_impl() const;
};

}

#endif
