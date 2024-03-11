#ifndef MIMIR_SEARCH_EVENT_HANDLERS_DEBUG_HPP_
#define MIMIR_SEARCH_EVENT_HANDLERS_DEBUG_HPP_

#include "mimir/common/printers.hpp"
#include "mimir/search/event_handlers/interface.hpp"

namespace mimir
{

/**
 * ID class to dispatch a specialized implementation
 */
struct DebugEventHandlerTag : public EventHandlerTag
{
};

/**
 * Implementation class
 */
template<>
class EventHandler<EventHandlerDispatcher<DebugEventHandlerTag>> : public EventHandlerBase<EventHandler<EventHandlerDispatcher<DebugEventHandlerTag>>>
{
private:
    /* Implement EventHandlerBase interface */
    template<typename>
    friend class EventHandlerBase;

    template<IsActionDispatcher A, IsStateDispatcher S>
    void on_generate_state_impl(ConstView<A> action, ConstView<S> successor_state, const PDDLFactories& pddl_factories) const
    {
        std::cout << "Action: " << std::make_tuple(action, std::cref(pddl_factories)) << std::endl;
        std::cout << "Successor: " << std::make_tuple(successor_state, std::cref(pddl_factories)) << std::endl;
    }

    template<IsStateDispatcher S>
    void on_expand_state_impl(ConstView<S> state, const PDDLFactories& pddl_factories) const
    {
        std::cout << "---" << std::endl;
        std::cout << "State: " << std::make_tuple(state, std::cref(pddl_factories)) << std::endl;
    }

    template<IsStateDispatcher S>
    void on_start_search_impl(ConstView<S> initial_state, const PDDLFactories& pddl_factories) const
    {
        std::cout << "Initial: " << std::make_tuple(initial_state, std::cref(pddl_factories)) << std::endl;
    }

    void on_end_search_impl() const
    {
        std::cout << "Num expanded states: " << this->m_statistics.get_num_expanded() << "\n"
                  << "Num generated states: " << this->m_statistics.get_num_generated() << "\n"
                  << "Search time: " << this->m_statistics.get_search_time_ms() << std::endl;
    }

    template<IsActionDispatcher A>
    void on_solved_impl(const std::vector<ConstView<A>>& ground_action_plan) const
    {
        std::cout << "Solved!" << std::endl;
    }

    void on_exhausted_impl() const { std::cout << "Exhausted!" << std::endl; }
};

}

#endif
