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
class SSGBase : public UncopyableMixin<SSGBase<Derived>> {
private:
    using P = typename TypeTraits<Derived>::PlanningModeTag;
    using S = typename TypeTraits<Derived>::StateTag;
    using A = typename TypeTraits<Derived>::ActionTag;
    using StateView = View<WrappedStateTag<S, P>>;
    using ActionView = View<WrappedActionTag<A, P, S>>;

    SSGBase() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

protected:
    UnorderedSet<WrappedStateTag<S, P>> m_states;

    Builder<WrappedStateTag<S, P>> m_state_builder;

public:
    [[nodiscard]] StateView get_or_create_initial_state(Problem problem) {
        return self().get_or_create_initial_state_impl(problem);
    }

    [[nodiscard]] StateView get_or_create_successor_state(StateView state, ActionView action) {
        return self().get_or_create_successor_state_impl(state, action);
    }
};

/**
 * ID class.
 *
 * Derive from it to provide your own implementation of a successor state generator.
*/
struct SSGBaseTag {};

template<typename DerivedTag>
concept IsSSGTag = std::derived_from<DerivedTag, SSGBaseTag>;


/**
 * Wrapper class.
 *
 * Wrap the tag and the planning mode to be able use a given planning mode.
*/
template<IsSSGTag SG, IsPlanningModeTag P, IsStateTag S, IsActionTag A>
struct WrappedSSGTag {};

template<typename T>
struct is_wrapped_ssg_tag : std::false_type {};

template<IsSSGTag SSG, IsPlanningModeTag P, IsStateTag S, IsActionTag A>
struct is_wrapped_ssg_tag<WrappedSSGTag<SSG, P, S, A>> : std::true_type {};

template<typename T>
concept IsWrappedSSG = is_wrapped_ssg_tag<T>::value;


/**
 * General implementation class.
 *
 * Spezialize the wrapped tag to provide your own implementation of a successor state generator.
*/
template<IsWrappedSSG S>
class SSG : public SSGBase<SSG<S>> { };


}  // namespace mimir

#endif  // MIMIR_SEARCH_SUCCESSOR_STATE_GENERATORS_TEMPLATE_HPP_
