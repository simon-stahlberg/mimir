#ifndef MIMIR_SEARCH_EVENT_HANDLERS_INTERFACE_HPP_
#define MIMIR_SEARCH_EVENT_HANDLERS_INTERFACE_HPP_

#include "mimir/formalism/factories.hpp"
#include "mimir/search/actions.hpp"
#include "mimir/search/states.hpp"
#include "mimir/search/statistics.hpp"

#include <chrono>
#include <concepts>

namespace mimir
{

/**
 * Interface class
 */
template<typename Derived>
class IEventHandler
{
private:
    IEventHandler() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

public:
    /// @brief React on generating a successor_state by applying an action.
    template<IsActionDispatcher A, IsStateDispatcher S>
    void on_generate_state(ConstView<A> action, ConstView<S> successor_state, const PDDLFactories& pddl_factories)
    {
        self().on_generate_state_base(action, successor_state, pddl_factories);
    }

    /// @brief React on expanding a state.
    template<IsStateDispatcher S>
    void on_expand_state(ConstView<S> state, const PDDLFactories& pddl_factories)
    {
        self().on_expand_state_base(state, pddl_factories);
    }

    /// @brief React on starting a search.
    template<IsStateDispatcher S>
    void on_start_search(ConstView<S> initial_state, const PDDLFactories& pddl_factories)
    {
        self().on_start_search_base(initial_state, pddl_factories);
    }

    /// @brief React on ending a search.
    void on_end_search() { self().on_end_search_base(); }

    /// @brief React on solving a search.
    template<IsActionDispatcher A>
    void on_solved(const std::vector<ConstView<A>>& ground_action_plan)
    {
        self().on_solved_base(ground_action_plan);
    }

    /// @brief React on exhausting a search.
    void on_exhausted() { self().on_exhausted_base(); }
};

/**
 * Base class
 */
template<typename Derived>
class EventHandlerBase : public IEventHandler<EventHandlerBase<Derived>>
{
protected:
    Statistics m_statistics;

private:
    EventHandlerBase() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

    /* Implement IEventHandler interface */
    template<typename>
    friend class IEventHandler;

    /// @brief React on generating a successor_state by applying an action.
    template<IsActionDispatcher A, IsStateDispatcher S>
    void on_generate_state_base(ConstView<A> action, ConstView<S> successor_state, const PDDLFactories& pddl_factories)
    {
        m_statistics.increment_num_generated();

        self().on_generate_state_impl(action, successor_state, pddl_factories);
    }

    /// @brief React on expanding a state.
    template<IsStateDispatcher S>
    void on_expand_state_base(ConstView<S> state, const PDDLFactories& pddl_factories)
    {
        m_statistics.increment_num_expanded();

        self().on_expand_state_impl(state, pddl_factories);
    }

    /// @brief React on starting a search.
    template<IsStateDispatcher S>
    void on_start_search_base(ConstView<S> initial_state, const PDDLFactories& pddl_factories)
    {
        m_statistics.set_search_start_time_point(std::chrono::high_resolution_clock::now());

        self().on_start_search_impl(initial_state, pddl_factories);
    }

    /// @brief React on ending a search.
    void on_end_search_base()
    {
        m_statistics.set_search_end_time_point(std::chrono::high_resolution_clock::now());

        self().on_end_search_impl();
    }

    /// @brief React on solving a search.
    template<IsActionDispatcher A>
    void on_solved_base(const std::vector<ConstView<A>>& ground_action_plan)
    {
        self().on_solved_impl(ground_action_plan);
    }

    /// @brief React on exhausting a search.
    void on_exhausted_base() { self().on_exhausted_impl(); }

    /// @brief Get the statistics.
    const Statistics& get_statistics() const { return m_statistics; }
};

/**
 * ID base class.
 *
 * Derive from it to provide your own implementation.
 *
 * Define new template parameters to your derived tag
 * in the declaration file of your derived class.
 */
struct EventHandlerTag
{
};

template<class DerivedTag>
concept IsEventHandlerTag = std::derived_from<DerivedTag, EventHandlerTag>;

/**
 * Dispatcher class.
 *
 * Wrap the tag to dispatch the correct overload.
 * The template parameters are arguments that all specializations have in common.
 * Do not add your specialized arguments here, add them to your derived tag instead.
 */
template<IsEventHandlerTag E>
struct EventHandlerDispatcher
{
};

template<typename T>
struct is_event_handler_dispatcher : std::false_type
{
};

template<IsEventHandlerTag E>
struct is_event_handler_dispatcher<EventHandlerDispatcher<E>> : std::true_type
{
};

template<typename T>
concept IsEventHandlerDispatcher = is_event_handler_dispatcher<T>::value;

/**
 * General implementation class.
 *
 * Specialize it with your dispatcher.
 */
template<IsEventHandlerDispatcher E>
class EventHandler : public EventHandlerBase<EventHandler<E>>
{
};

}

#endif