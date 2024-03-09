#ifndef MIMIR_SEARCH_EVENT_HANDLERS_INTERFACE_HPP_
#define MIMIR_SEARCH_EVENT_HANDLERS_INTERFACE_HPP_

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
    void on_generate_state(/* state, creating_action, succ_state? */) const { self().on_generate_state_impl(); }

    void on_expand_state(/* state? */) const { self().on_expand_state_impl(); }

    void on_start_search(/* initial_state? */) const { self().on_start_search_impl(); }

    void on_finish_search(/* statistics? */) const { self().on_finish_search_impl(); }

    void on_solved(/* plan? */) const { self().on_solved_impl(); }

    void on_unsolvable(/**/) const { self().on_unsolvable_impl(); }
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
class EventHandler : public IEventHandler<EventHandler<E>>
{
};

}

#endif