#ifndef MIMIR_SEARCH_EVENT_HANDLERS_MINIMAL_HPP_
#define MIMIR_SEARCH_EVENT_HANDLERS_MINIMAL_HPP_

#include "mimir/common/printers.hpp"
#include "mimir/common/translations.hpp"
#include "mimir/search/event_handlers/interface.hpp"

#include <iostream>

namespace mimir
{

/**
 * Implementation class
 */
class MinimalEventHandler : public EventHandlerBase<MinimalEventHandler>
{
private:
    /* Implement EventHandlerBase interface */
    friend class EventHandlerBase<MinimalEventHandler>;

    void on_generate_state_impl(ConstView<ActionDispatcher<StateReprTag>> action,
                                ConstView<StateDispatcher<StateReprTag>> successor_state,
                                const PDDLFactories& pddl_factories) const
    {
    }

    void on_expand_state_impl(ConstView<StateDispatcher<StateReprTag>> state, const PDDLFactories& pddl_factories) const {}

    void on_start_search_impl(ConstView<StateDispatcher<StateReprTag>> initial_state, const PDDLFactories& pddl_factories) const {}

    void on_end_search_impl() const
    {
        std::cout << "Num expanded states: " << this->m_statistics.get_num_expanded() << "\n"
                  << "Num generated states: " << this->m_statistics.get_num_generated() << "\n"
                  << "Search time: " << this->m_statistics.get_search_time_ms().count() << "ms" << std::endl;
    }

    void on_solved_impl(const std::vector<ConstView<ActionDispatcher<StateReprTag>>>& ground_action_plan) const
    {
        auto plan = to_plan(ground_action_plan);
        std::cout << "Plan found with cost: " << plan.get_cost() << std::endl;
        for (size_t i = 0; i < plan.get_actions().size(); ++i)
        {
            std::cout << i + 1 << ". " << plan.get_actions()[i] << std::endl;
        }
    }

    void on_exhausted_impl() const { std::cout << "Exhausted!" << std::endl; }
};

}

#endif
