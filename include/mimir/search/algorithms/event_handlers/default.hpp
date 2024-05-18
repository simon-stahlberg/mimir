#ifndef MIMIR_SEARCH_ALGORITHMS_EVENT_HANDLERS_MINIMAL_HPP_
#define MIMIR_SEARCH_ALGORITHMS_EVENT_HANDLERS_MINIMAL_HPP_

#include "mimir/common/printers.hpp"
#include "mimir/search/algorithms/event_handlers/interface.hpp"
#include "mimir/search/plan.hpp"

#include <iostream>

namespace mimir
{

/**
 * Implementation class
 */
class DefaultAlgorithmEventHandler : public AlgorithmEventHandlerBase<DefaultAlgorithmEventHandler>
{
private:
    /* Implement AlgorithmEventHandlerBase interface */
    friend class AlgorithmEventHandlerBase<DefaultAlgorithmEventHandler>;

    void on_generate_state_impl(GroundAction action, State successor_state, const PDDLFactories& pddl_factories) const {}

    void on_finish_g_layer_impl(uint64_t g_value, uint64_t num_states) const
    {
        std::cout << "[Algorithm] Finished state expansion until g-layer " << g_value << " with num states " << num_states << std::endl;
    }

    void on_expand_state_impl(State state, const PDDLFactories& pddl_factories) const {}

    void on_start_search_impl(State initial_state, const PDDLFactories& pddl_factories) const
    {  //
        std::cout << "[Algorithm] Search started." << std::endl;
    }

    void on_end_search_impl() const
    {
        std::cout << "[Algorithm] Search finished."
                  << "\n"
                  << "[Algorithm] Num expanded states: " << this->m_statistics.get_num_expanded() << "\n"
                  << "[Algorithm] Num generated states: " << this->m_statistics.get_num_generated() << "\n"
                  << "[Algorithm] Search time: " << this->m_statistics.get_search_time_ms().count() << "ms" << std::endl;
    }

    void on_solved_impl(const GroundActionList& ground_action_plan) const
    {
        auto plan = to_plan(ground_action_plan);
        std::cout << "[Algorithm] Plan found with cost: " << plan.get_cost() << std::endl;
        for (size_t i = 0; i < plan.get_actions().size(); ++i)
        {
            std::cout << i + 1 << ". " << plan.get_actions()[i] << std::endl;
        }
    }

    void on_exhausted_impl() const { std::cout << "[Algorithm] Exhausted!" << std::endl; }
};

}

#endif
