#ifndef MIMIR_SEARCH_STATES_INTERFACE_HPP_
#define MIMIR_SEARCH_STATES_INTERFACE_HPP_

#include "mimir/search/builder.hpp"
#include "mimir/search/states/tags.hpp"
#include "mimir/search/view_const.hpp"

#include <concepts>
#include <cstdint>

namespace mimir
{

/**
 * Interface class
 */
template<typename Derived>
class IStateBuilder
{
private:
    IStateBuilder() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

public:
    [[nodiscard]] uint32_t& get_id() { return self().get_id_impl(); }
};

template<typename Derived>
class IStateView
{
private:
    IStateView() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

public:
    /**
     * Accessors
     */

    [[nodiscard]] uint32_t get_id() const { return self().get_id_impl(); }

    /**
     * Iterators
     */

    [[nodiscard]] auto begin() const { return self().begin_impl(); }
    [[nodiscard]] auto end() const { return self().end_impl(); }
};

/**
 * General implementation class.
 *
 * Specialize the wrapped tag to provide your own implementation of a state representation.
 */
template<IsStateDispatcher S>
class Builder<S> : public IBuilder<Builder<S>>, public IStateBuilder<Builder<S>>
{
};

template<IsStateDispatcher S>
class ConstView<S> : public IConstView<ConstView<S>>, public IStateView<ConstView<S>>
{
};

}

#endif
