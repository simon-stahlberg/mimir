#ifndef MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_INTERFACE_HPP_
#define MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_INTERFACE_HPP_

#include "../../formalism/common/types.hpp"
#include "../../formalism/problem/declarations.hpp"
#include "../actions.hpp"
#include "../states.hpp"
#include "../type_traits.hpp"

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

    using ConstStateView = ConstView<StateDispatcher<S, P>>;
    using ConstActionView = ConstView<ActionDispatcher<P, S>>;

    IAAG() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

public:
    /// @brief Generate all applicable actions for a given state.
    void generate_applicable_actions(ConstStateView state, std::vector<ConstActionView>& out_applicable_actions)
    {
        self().generate_applicable_actions_impl(state, out_applicable_actions);
    }
};

/**
 * Dispatcher class.
 *
 * Wrap the tag to dispatch the correct overload.
 * The template parameters are arguments that all specializations have in common.
 * Do not add your specialized arguments here, add them to your derived tag instead.
 */
template<IsPlanningModeTag P, IsStateTag S>
struct AAGDispatcher
{
};

template<typename T>
struct is_aag_dispatcher : std::false_type
{
};

template<IsPlanningModeTag P, IsStateTag S>
struct is_aag_dispatcher<AAGDispatcher<P, S>> : std::true_type
{
};

template<typename T>
concept IsAAGDispatcher = is_aag_dispatcher<T>::value;

/**
 * General implementation class.
 *
 * Specialize it with your dispatcher.
 */
template<IsAAGDispatcher A>
class AAG : public IAAG<AAG<A>>
{
};

/**
 * Type traits.
 */
template<IsPlanningModeTag P, IsStateTag S>
struct TypeTraits<AAG<AAGDispatcher<P, S>>>
{
    using PlanningModeTag = P;
    using StateTag = S;
};

}

#endif
