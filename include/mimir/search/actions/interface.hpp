#ifndef MIMIR_SEARCH_ACTIONS_INTERFACE_HPP_
#define MIMIR_SEARCH_ACTIONS_INTERFACE_HPP_

#include "mimir/search/builder.hpp"
#include "mimir/search/config.hpp"
#include "mimir/search/states.hpp"
#include "mimir/search/type_traits.hpp"
#include "mimir/search/view_const.hpp"

namespace mimir
{

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
    [[nodiscard]] std::string str() const { return self().str_impl(); }
};

/**
 * Dispatcher class.
 *
 * The template parameters are arguments that all specializations have in common.
 * Do not add your specialized arguments here, add them to your derived tag instead.
 */
template<IsPlanningModeTag P, IsStateTag S>
struct ActionDispatcher
{
};

template<typename T>
struct is_action_dispatcher : std::false_type
{
};

template<IsPlanningModeTag P, IsStateTag S>
struct is_action_dispatcher<ActionDispatcher<P, S>> : std::true_type
{
};

template<typename T>
concept IsActionDispatcher = is_action_dispatcher<T>::value;

/**
 * Type traits.
 */
template<IsPlanningModeTag P, IsStateTag S>
struct TypeTraits<Builder<ActionDispatcher<P, S>>>
{
    using PlanningModeTag = P;
    using StateTag = S;
};

template<IsPlanningModeTag P, IsStateTag S>
struct TypeTraits<ConstView<ActionDispatcher<P, S>>>
{
    using PlanningModeTag = P;
    using StateTag = S;
};

}

#endif