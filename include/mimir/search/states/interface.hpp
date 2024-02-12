#ifndef MIMIR_SEARCH_STATES_INTERFACE_HPP_
#define MIMIR_SEARCH_STATES_INTERFACE_HPP_

#include "../config.hpp"
#include "../type_traits.hpp"


namespace mimir
{


/**
 * ID base class.
 *
 * Derive from it to provide your own implementation.
 *
 * Define new template parameters to your derived tag
 * in the declaration file of your derived class.
*/
struct StateTag {};

template<typename DerivedTag>
concept IsStateTag = std::derived_from<DerivedTag, StateTag>;


/**
 * Interface class
*/
template<typename Derived>
class IStateBuilder
{
private:
    using P = typename TypeTraits<Derived>::PlanningModeTag;

    IStateBuilder() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

public:
    void set_id(uint32_t id) { self().set_id_impl(id); }
};


template<typename Derived>
class IStateView
{
private:
    using P = typename TypeTraits<Derived>::PlanningModeTag;

    IStateView() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

public:
    [[nodiscard]] uint32_t get_id() const { return self().get_id_impl(); }
};


/**
 * Dispatcher class.
 *
 * Wrap the tag to dispatch the correct overload.
 * The template parameters are arguments that all specializations have in common.
 * Do not add your specialized arguments here, add them to your derived tag instead.
*/
template<IsStateTag S, IsPlanningModeTag P>
struct StateDispatcher {};

template<typename T>
struct is_state_dispatcher : std::false_type {};

template<IsStateTag S, IsPlanningModeTag P>
struct is_state_dispatcher<StateDispatcher<S, P>> : std::true_type {};

template<typename T>
concept IsStateDispatcher = is_state_dispatcher<T>::value;


/**
 * General implementation class.
 *
 * Specialize the wrapped tag to provide your own implementation of a state representation.
*/
template<IsStateDispatcher S>
class Builder<S> : public IBuilderBase<Builder<S>>, public IStateBuilder<Builder<S>> {};

template<IsStateDispatcher S>
class View<S> : public IView<View<S>>, public IStateView<View<S>> {};


}

#endif
