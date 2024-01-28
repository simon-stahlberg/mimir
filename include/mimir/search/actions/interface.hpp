#ifndef MIMIR_SEARCH_ACTIONS_INTERFACE_HPP_
#define MIMIR_SEARCH_ACTIONS_INTERFACE_HPP_

#include "../config.hpp"
#include "../states.hpp"
#include "../type_traits.hpp"

#include "../../buffer/view_base.hpp"
#include "../../buffer/byte_stream_utils.hpp"


namespace mimir {


/**
 * Interface class
*/
template<typename Derived>
class IActionBuilder
{
private:
    using P = typename TypeTraits<Derived>::PlanningModeTag;
    using S = typename TypeTraits<Derived>::StateTag;

    IActionBuilder() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

public:
};


/**
 * Interface class
*/
template<typename Derived>
class IActionView
{
private:
    using P = typename TypeTraits<Derived>::PlanningModeTag;
    using S = typename TypeTraits<Derived>::StateTag;

    IActionView() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

public:
    /* Mutable getters. */

    /* Immutable getters. */
    std::string str() const { return self().str_impl(); }
};


/**
 * ID base class.
 *
 * Derive from it to provide your own implementation.
 *
 * Define new template parameters to your derived tag
 * in the declaration file of your derived class.
*/
struct ActionTag {};

template<typename DerivedTag>
concept IsActionTag = std::derived_from<DerivedTag, ActionTag>;


/**
 * Dispatcher class.
 *
 * Wrap the tag to dispatch the correct overload.
 * The template parameters are arguments that all specializations have in common.
 * Do not add your specialized arguments here, add them to your derived tag instead.
*/
template<IsActionTag A, IsPlanningModeTag P, IsStateTag S>
struct ActionDispatcher {};

template<typename T>
struct is_action_dispatcher : std::false_type {};

template<IsActionTag A, IsPlanningModeTag P, IsStateTag S>
struct is_action_dispatcher<ActionDispatcher<A, P, S>> : std::true_type {};

template<typename T>
concept IsActionDispatcher = is_action_dispatcher<T>::value;


}


#endif