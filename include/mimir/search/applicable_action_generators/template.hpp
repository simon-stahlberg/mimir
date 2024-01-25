#ifndef MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_TEMPLATE_HPP_
#define MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_TEMPLATE_HPP_

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
class AAGBase : public UncopyableMixin<AAGBase<Derived>> {
private:
    using P = typename TypeTraits<Derived>::PlanningModeTag;
    using S = typename TypeTraits<Derived>::StateTag;
    using A = typename TypeTraits<Derived>::ActionTag;

    using StateView = View<StateDispatcher<S, P>>;
    using ActionView = View<ActionDispatcher<A, P, S>>;


    AAGBase() = default;
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
struct AAGBaseTag {};

template<typename DerivedTag>
concept IsAAGTag = std::derived_from<DerivedTag, AAGBaseTag>;


/**
 * Dispatcher class.
 *
 * Wrap the tag and variable number of template arguments.
 * 
 * Define required input template parameters using SFINAE
 * in the declaration file of your derived class.
*/
template<IsAAGTag AAG, typename... Ts>
struct AAGDispatcher {};

template<typename T>
struct is_aag_dispatcher : std::false_type {};

template<typename T>
concept IsAAGDispatcher = is_aag_dispatcher<T>::value;


/**
 * General implementation class.
 *
 * Spezialize it using our wrapper dispatch class.
 * in the spezialization file of your derived class.
*/
template<IsAAGDispatcher A>
class AAG : public AAGBase<AAG<A>> { 

};



}  // namespace mimir

#endif  // MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_TEMPLATE_HPP_
