#ifndef MIMIR_SEARCH_AXIOMS_INTERFACE_HPP_
#define MIMIR_SEARCH_AXIOMS_INTERFACE_HPP_

#include "mimir/search/axioms/tags.hpp"
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
};

/**
 * General implementation class.
 *
 * Specialize the wrapped tag to provide your own implementation of a state representation.
 */
template<IsAxiomDispatcher A>
class Builder<A> : public IBuilder<Builder<A>>, public IAxiomBuilder<Builder<A>>
{
};

template<IsAxiomDispatcher A>
class ConstView<A> : public IConstView<ConstView<A>>, public IAxiomView<ConstView<A>>
{
};

}

#endif