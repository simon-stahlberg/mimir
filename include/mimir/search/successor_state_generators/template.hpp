#ifndef MIMIR_SEARCH_SUCCESSOR_STATE_GENERATORS_TEMPLATE_HPP_
#define MIMIR_SEARCH_SUCCESSOR_STATE_GENERATORS_TEMPLATE_HPP_

#include "../config.hpp"
#include "../grounded/state_view.hpp"
#include "../grounded/state_builder.hpp"
#include "../lifted/state_view.hpp"
#include "../lifted/state_builder.hpp"
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

    SSGBase() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

protected:
    UnorderedSet<State<P>> m_states;

    Builder<State<P>> m_state_builder;

public:
    [[nodiscard]] View<State<P>> get_or_create_initial_state(Problem problem) {
        return self().get_or_create_initial_state_impl(problem);
    }

    [[nodiscard]] View<State<P>> get_or_create_successor_state(View<State<P>> state, GroundAction action) {
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
template<IsSSGTag SG, IsPlanningModeTag P>
struct WrappedSSGTag {
    using PlanningModeTag = P;
    using SSGTag = SG;
};

template<typename T>
concept IsWrappedSSG = requires {
    typename T::PlanningModeTag;
    typename T::SSGTag;
};


/**
 * General implementation class.
 *
 * Spezialize the wrapped tag to provide your own implementation of a successor state generator.
*/
template<IsWrappedSSG S>
class SSG : public SSGBase<SSG<S>> { };


}  // namespace mimir

#endif  // MIMIR_SEARCH_SUCCESSOR_STATE_GENERATORS_TEMPLATE_HPP_
