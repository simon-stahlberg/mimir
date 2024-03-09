#ifndef MIMIR_SEARCH_SUCCESSOR_STATE_GENERATORS_INTERFACE_HPP_
#define MIMIR_SEARCH_SUCCESSOR_STATE_GENERATORS_INTERFACE_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/search/actions.hpp"
#include "mimir/search/config.hpp"
#include "mimir/search/states.hpp"
#include "mimir/search/type_traits.hpp"

#include <cstddef>

namespace mimir
{

/**
 * Interface class
 */
template<typename Derived>
class ISSG
{
private:
    using P = typename TypeTraits<Derived>::PlanningModeTag;
    using S = typename TypeTraits<Derived>::StateTag;
    using ConstStateView = ConstView<StateDispatcher<S, P>>;
    using ConstActionView = ConstView<ActionDispatcher<P, S>>;

    ISSG() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

public:
    [[nodiscard]] ConstStateView get_or_create_initial_state(Problem problem) { return self().get_or_create_initial_state_impl(problem); }

    [[nodiscard]] ConstStateView get_or_create_successor_state(ConstStateView state, ConstActionView action)
    {
        return self().get_or_create_successor_state_impl(state, action);
    }

    [[nodiscard]] size_t state_count() const { return self().state_count_impl(); }
};

/**
 * Dispatcher class.
 *
 * Wrap the tag to dispatch the correct overload.
 * The template parameters are arguments that all specializations have in common.
 * Do not add your specialized arguments here, add them to your derived tag instead.
 */
template<IsPlanningModeTag P, IsStateTag S>
struct SSGDispatcher
{
};

template<typename T>
struct is_ssg_dispatcher : std::false_type
{
};

template<IsPlanningModeTag P, IsStateTag S>
struct is_ssg_dispatcher<SSGDispatcher<P, S>> : std::true_type
{
};

template<typename T>
concept IsSSGDispatcher = is_ssg_dispatcher<T>::value;

/**
 * General implementation class.
 *
 * Specialize the wrapped tag to provide your own implementation of a successor state generator.
 */
template<IsSSGDispatcher S>
class SSG : public ISSG<SSG<S>>
{
};

/**
 * Type traits.
 */
template<IsPlanningModeTag P, IsStateTag S>
struct TypeTraits<SSG<SSGDispatcher<P, S>>>
{
    using PlanningModeTag = P;
    using StateTag = S;
};

}

#endif
