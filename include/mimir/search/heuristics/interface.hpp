#ifndef MIMIR_SEARCH_HEURISTICS_INTERFACE_HPP_
#define MIMIR_SEARCH_HEURISTICS_INTERFACE_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/search/heuristics/tags.hpp"
#include "mimir/search/states.hpp"

namespace mimir
{

/**
 * Interface class
 */
class IDynamicHeuristic
{
public:
    virtual ~IDynamicHeuristic() = default;

    [[nodiscard]] virtual double compute_heuristic(State state) = 0;
};

/**
 * Interface class
 */
template<typename Derived>
class IStaticHeuristic : public IDynamicHeuristic
{
private:
    using S = typename TypeTraits<Derived>::StateTag;
    using StateRepr = ConstView<StateDispatcher<S>>;

    IStaticHeuristic() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

public:
    [[nodiscard]] double compute_heuristic(StateRepr state) override { return self().compute_heuristic_impl(state); }
};

/**
 * General implementation class.
 *
 * Specialize it with your dispatcher.
 */
template<IsHeuristicDispatcher T>
class Heuristic : public IStaticHeuristic<Heuristic<T>>
{
};

/**
 * Type traits.
 */
template<IsHeuristicTag H>
struct TypeTraits<Heuristic<HeuristicDispatcher<H, DenseStateTag>>>
{
    using StateTag = DenseStateTag;
};

}

#endif