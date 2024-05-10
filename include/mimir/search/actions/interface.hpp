#ifndef MIMIR_SEARCH_ACTIONS_INTERFACE_HPP_
#define MIMIR_SEARCH_ACTIONS_INTERFACE_HPP_

#include "mimir/search/actions/tags.hpp"
#include "mimir/search/states.hpp"

namespace mimir
{

/**
 * Interface class
 */
template<typename Derived>
class IActionBuilder
{
private:
    IActionBuilder() = default;
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
class IActionView
{
private:
    IActionView() = default;
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
 * Type traits.
 */
template<IsStateTag S>
struct TypeTraits<Builder<ActionDispatcher<S>>>
{
    using StateTag = S;
};

template<IsStateTag S>
struct TypeTraits<ConstView<ActionDispatcher<S>>>
{
    using StateTag = S;
};

}

#endif