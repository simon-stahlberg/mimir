#ifndef MIMIR_SEARCH_SUCCESSOR_STATE_GENERATORS_TEMPLATE_HPP_
#define MIMIR_SEARCH_SUCCESSOR_STATE_GENERATORS_TEMPLATE_HPP_

#include "../config.hpp"
#include "../states.hpp"
#include "../actions.hpp"
#include "../type_traits.hpp"

#include "../../buffer/containers/unordered_set.hpp"
#include "../../common/mixins.hpp"
#include "../../formalism/problem/declarations.hpp"


namespace mimir
{

/**
 * Interface class
*/
template<typename Derived>
class SSGBase {
private:
    using P = typename TypeTraits<Derived>::PlanningModeTag;
    using S = typename TypeTraits<Derived>::StateTag;
    using A = typename TypeTraits<Derived>::ActionTag;
    using StateView = View<StateDispatcher<S, P>>;
    using ActionView = View<ActionDispatcher<A, P, S>>;

    SSGBase() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

protected:
    UnorderedSet<StateDispatcher<S, P>> m_states;

    Builder<StateDispatcher<S, P>> m_state_builder;

public:
    [[nodiscard]] StateView get_or_create_initial_state(Problem problem) {
        return self().get_or_create_initial_state_impl(problem);
    }

    [[nodiscard]] StateView get_or_create_successor_state(StateView state, ActionView action) {
        return self().get_or_create_successor_state_impl(state, action);
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
struct SSGBaseTag {};

template<typename DerivedTag>
concept IsSSGTag = std::derived_from<DerivedTag, SSGBaseTag>;


/**
 * Dispatcher class.
 *
 * Wrap the tag to dispatch the correct overload.
 * The template parameters are arguments that all specializations have in common.
 * Do not add your specialized arguments here, add them to your derived tag instead.
*/
template<IsSSGTag SG, IsPlanningModeTag P, IsStateTag S, IsActionTag A>
struct SSGDispatcher {};

template<typename T>
struct is_ssg_dispatcher : std::false_type {};

template<IsSSGTag SSG, IsPlanningModeTag P, IsStateTag S, IsActionTag A>
struct is_ssg_dispatcher<SSGDispatcher<SSG, P, S, A>> : std::true_type {};

template<typename T>
concept IsSSGDispatcher = is_ssg_dispatcher<T>::value;


/**
 * General implementation class.
 *
 * Specialize the wrapped tag to provide your own implementation of a successor state generator.
*/
template<IsSSGDispatcher S>
class SSG : public SSGBase<SSG<S>> { };


}  // namespace mimir

#endif  // MIMIR_SEARCH_SUCCESSOR_STATE_GENERATORS_TEMPLATE_HPP_
