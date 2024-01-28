#ifndef MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_INTERFACE_HPP_
#define MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_INTERFACE_HPP_

#include "../actions.hpp"
#include "../states.hpp"
#include "../type_traits.hpp"

#include "../../formalism/problem/declarations.hpp"


namespace mimir
{

/**
 * Interface class.
*/
template<typename Derived>
class IAAG
{
private:
    using P = typename TypeTraits<Derived>::PlanningModeTag;
    using S = typename TypeTraits<Derived>::StateTag;
    using A = typename TypeTraits<Derived>::ActionTag;

    using StateView = View<StateDispatcher<S, P>>;
    using ActionView = View<ActionDispatcher<A, P, S>>;

    IAAG() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

public:
    /// @brief Generate all applicable actions for a given state.
    void generate_applicable_actions(StateView state, std::vector<ActionView>& out_applicable_actions) {
        self().generate_applicable_actions_impl(state, out_applicable_actions);
    }
};


/**
 * ID base class.
 *
 * Derive from it to provide your own implementation.
 *
 * Define new template parameters to your derived tag
 * in the declaration file of your derived class.
*/
struct AAGTag {};

template<typename DerivedTag>
concept IsAAGTag = std::derived_from<DerivedTag, AAGTag>;


/**
 * Dispatcher class.
 *
 * Wrap the tag to dispatch the correct overload.
 * The template parameters are arguments that all specializations have in common.
 * Do not add your specialized arguments here, add them to your derived tag instead.
*/
template<IsAAGTag AAG, IsPlanningModeTag P, IsStateTag S, IsActionTag A>
struct AAGDispatcher {};

template<typename T>
struct is_aag_dispatcher : std::false_type {};

template<IsAAGTag AAG, IsPlanningModeTag P, IsStateTag S, IsActionTag A>
struct is_aag_dispatcher<AAGDispatcher<AAG, P, S, A>> : std::true_type {};

template<typename T>
concept IsAAGDispatcher = is_aag_dispatcher<T>::value;


/**
 * General implementation class.
 *
 * Specialize it with your dispatcher.
*/
template<IsAAGDispatcher A>
class AAG : public IAAG<AAG<A>> {};


}

#endif