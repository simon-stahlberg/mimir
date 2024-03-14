#ifndef MIMIR_SEARCH_EVENT_HANDLERS_DEBUG_HPP_
#define MIMIR_SEARCH_EVENT_HANDLERS_DEBUG_HPP_

#include "mimir/common/printers.hpp"
#include "mimir/common/translations.hpp"
#include "mimir/search/event_handlers/interface.hpp"

#include <iostream>

namespace mimir
{

/**
 * Implementation class
 */
class DebugEventHandler : public EventHandlerBase<DebugEventHandler>
{
private:
    /* Implement EventHandlerBase interface */
    template<typename>
    friend class EventHandlerBase;

    void on_generate_state_impl(ConstView<ActionDispatcher<StateReprTag>> action,
                                ConstView<StateDispatcher<StateReprTag>> successor_state,
                                const PDDLFactories& pddl_factories) const
    {
        std::cout << "Action: " << std::make_tuple(action, std::cref(pddl_factories)) << std::endl;
        std::cout << "Successor: " << std::make_tuple(successor_state, std::cref(pddl_factories)) << std::endl;
    }

    void on_expand_state_impl(ConstView<StateDispatcher<StateReprTag>> state, const PDDLFactories& pddl_factories) const
    {
        std::cout << "---" << std::endl;
        std::cout << "State: " << std::make_tuple(state, std::cref(pddl_factories)) << std::endl;
    }

    void on_start_search_impl(ConstView<StateDispatcher<StateReprTag>> initial_state, const PDDLFactories& pddl_factories) const
    {
        std::cout << "Initial: " << std::make_tuple(initial_state, std::cref(pddl_factories)) << std::endl;
    }

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
