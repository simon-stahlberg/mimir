#ifndef MIMIR_SEARCH_EVENT_HANDLERS_DEBUG_HPP_
#define MIMIR_SEARCH_EVENT_HANDLERS_DEBUG_HPP_

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
class EventHandler<EventHandlerDispatcher<DebugEventHandlerTag>> : public IEventHandler<EventHandler<EventHandlerDispatcher<DebugEventHandlerTag>>>
{
private:
    /* Implement IEventHandler interface */
    template<typename>
    friend class IEventHandler;

    template<IsActionDispatcher A, IsStateDispatcher S>
    void on_generate_state_impl(ConstView<A> action, ConstView<S> successor_state) const
    {
    }

    template<IsStateDispatcher S>
    void on_expand_state_impl(ConstView<S> state) const
    {
    }

    template<IsStateDispatcher S>
    void on_start_search_impl(ConstView<S> initial_state) const
    {
    }

    void on_end_search_impl(const Statistics& statistics) const {}

    template<IsActionDispatcher A>
    void on_solved_impl(const std::vector<ConstView<A>>& ground_action_plan) const
    {
    }

    void on_exhausted_impl() const {}
};

}

#endif
