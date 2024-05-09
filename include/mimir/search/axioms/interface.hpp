#ifndef MIMIR_SEARCH_AXIOMS_INTERFACE_HPP_
#define MIMIR_SEARCH_AXIOMS_INTERFACE_HPP_

#include "mimir/search/builder.hpp"
#include "mimir/search/states.hpp"
#include "mimir/search/view_const.hpp"

namespace mimir
{

/**
 * Interface class
 */
template<typename Derived>
class IAxiomBuilder
{
private:
    IAxiomBuilder() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

public:
};

/**
 * Interface class
 */
template<typename Derived>
class IAxiomView
{
private:
    IAxiomView() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

public:
    /* Mutable getters. */

    /* Immutable getters. */
    [[nodiscard]] std::string str() const { return self().str_impl(); }
};

/**
 * Dispatcher class.
 *
 * The template parameters are arguments that all specializations have in common.
 * Do not add your specialized arguments here, add them to your derived tag instead.
 */
template<IsStateTag S>
struct AxiomDispatcher
{
};

template<typename T>
struct is_axiom_dispatcher : std::false_type
{
};

template<IsStateTag S>
struct is_axiom_dispatcher<AxiomDispatcher<S>> : std::true_type
{
};

template<typename T>
concept IsAxiomDispatcher = is_axiom_dispatcher<T>::value;

/**
 * Type traits.
 */
template<IsStateTag S>
struct TypeTraits<Builder<AxiomDispatcher<S>>>
{
    using StateTag = S;
};

template<IsStateTag S>
struct TypeTraits<ConstView<AxiomDispatcher<S>>>
{
    using StateTag = S;
};

}

#endif