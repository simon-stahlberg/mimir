#ifndef MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_TEMPLATE_HPP_
#define MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_TEMPLATE_HPP_

#include "../state.hpp"
#include "../grounded/state_view.hpp"
#include "../lifted/state_view.hpp"
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

    AAGBase() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

public:
    /// @brief Generate all applicable actions for a given state.
    void generate_applicable_actions(View<State<P>> state, GroundActionList& out_applicable_actions) {
        self().generate_applicable_actions_impl(state, out_applicable_actions);
    }
};


/**
 * ID dispatch class.
 *
 * Derive from it to provide your own implementation of a applicable action generator.
*/
struct AAGBaseTag {};

template<typename DerivedTag>
concept IsAAGTag = std::derived_from<DerivedTag, AAGBaseTag>;


/**
 * Wrapper class.
 *
 * Wrap the tag and the planning mode to be able use a given planning mode.
*/
template<IsAAGTag A, IsPlanningModeTag P>
struct WrappedAAGTag {
    using AAGTag = A;
    using PlanningModeTag = P;
};

template<typename T>
concept IsWrappedAAGTag = requires {
    typename T::PlanningModeTag;
    typename T::AAGTag;
};


/**
 * General implementation class.
 *
 * Spezialize the wrapped tag to provide your own implementation of a applicable action generator.
*/
template<IsWrappedAAGTag A>
class AAG : public AAGBase<AAG<A>> { };



}  // namespace mimir

#endif  // MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_TEMPLATE_HPP_
