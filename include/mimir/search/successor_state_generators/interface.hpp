#ifndef MIMIR_SEARCH_SUCCESSOR_STATE_GENERATORS_INTERFACE_HPP_
#define MIMIR_SEARCH_SUCCESSOR_STATE_GENERATORS_INTERFACE_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/search/compile_flags.hpp"

namespace mimir
{
/**
 * Dynamic interface class
 */
class IDynamicSSG
{
public:
    virtual ~IDynamicSSG() = default;

    [[nodiscard]] virtual State get_or_create_initial_state(Problem problem) = 0;

    [[nodiscard]] virtual State get_or_create_successor_state(State state, GroundAction action) = 0;

    [[nodiscard]] virtual size_t get_state_count() const = 0;
};

/**
 * Static interface class
 */
template<typename Derived>
class IStaticSSG : public IDynamicSSG
{
private:
    using S = typename TypeTraits<Derived>::StateTag;
    using StateRepr = ConstView<StateDispatcher<S>>;
    using GroundActionRepr = ConstView<ActionDispatcher<S>>;

    IStaticSSG() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

public:
    [[nodiscard]] StateRepr get_or_create_initial_state(Problem problem) override { return self().get_or_create_initial_state_impl(problem); }

    [[nodiscard]] StateRepr get_or_create_successor_state(StateRepr state, GroundActionRepr action) override
    {
        return self().get_or_create_successor_state_impl(state, action);
    }

    [[nodiscard]] size_t get_state_count() const override { return self().get_state_count_impl(); }
};

/**
 * Dispatcher class.
 *
 * Wrap the tag to dispatch the correct overload.
 * The template parameters are arguments that all specializations have in common.
 * Do not add your specialized arguments here, add them to your derived tag instead.
 */
template<IsStateTag S>
struct SSGDispatcher
{
};

template<typename T>
struct is_ssg_dispatcher : std::false_type
{
};

template<IsStateTag S>
struct is_ssg_dispatcher<SSGDispatcher<S>> : std::true_type
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
class SSG : public IStaticSSG<SSG<S>>
{
};

/**
 * Type traits.
 */
template<IsStateTag S>
struct TypeTraits<SSG<SSGDispatcher<S>>>
{
    using StateTag = S;
};
}

#endif
