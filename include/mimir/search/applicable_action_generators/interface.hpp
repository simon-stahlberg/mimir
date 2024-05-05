#ifndef MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_INTERFACE_HPP_
#define MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_INTERFACE_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/search/compile_flags.hpp"

namespace mimir
{

/**
 * Dynamic interface class.
 */
class IDynamicAAG
{
public:
    virtual ~IDynamicAAG() = default;

    /// @brief Generate all applicable actions for a given state.
    virtual void generate_applicable_actions(State state, std::vector<GroundAction>& out_applicable_actions) = 0;

    /// @brief Return the action with the given id.
    [[nodiscard]] virtual GroundAction get_action(size_t action_id) const = 0;
};

/**
 * Static interface class.
 */
template<typename Derived>
class IStaticAAG : public IDynamicAAG
{
private:
    using S = typename TypeTraits<Derived>::StateTag;
    using StateRepr = ConstView<StateDispatcher<S>>;
    using GroundActionRepr = ConstView<ActionDispatcher<S>>;

    IStaticAAG() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

public:
    /// @brief Generate all applicable actions for a given state.
    void generate_applicable_actions(const StateRepr state, std::vector<GroundActionRepr>& out_applicable_actions) override
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
template<IsStateTag S>
struct LiftedAAGDispatcher
{
};

template<IsStateTag S>
struct GroundedAAGDispatcher
{
};

template<typename T>
struct is_lifted_aag_dispatcher : std::false_type
{
};

template<typename T>
struct is_grounded_aag_dispatcher : std::false_type
{
};

template<IsStateTag S>
struct is_lifted_aag_dispatcher<LiftedAAGDispatcher<S>> : std::true_type
{
};

template<IsStateTag S>
struct is_grounded_aag_dispatcher<GroundedAAGDispatcher<S>> : std::true_type
{
};

template<typename T>
concept IsAAGDispatcher = is_lifted_aag_dispatcher<T>::value || is_grounded_aag_dispatcher<T>::value;

/**
 * General implementation class.
 *
 * Specialize it with your dispatcher.
 */
template<IsAAGDispatcher A>
class AAG : public IStaticAAG<AAG<A>>
{
};

/**
 * Type traits.
 */
template<IsStateTag S>
struct TypeTraits<AAG<LiftedAAGDispatcher<S>>>
{
    using StateTag = S;
};

template<IsStateTag S>
struct TypeTraits<AAG<GroundedAAGDispatcher<S>>>
{
    using StateTag = S;
};

}

#endif
