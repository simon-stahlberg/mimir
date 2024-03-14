#ifndef MIMIR_SEARCH_HEURISTICS_INTERFACE_HPP_
#define MIMIR_SEARCH_HEURISTICS_INTERFACE_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/search/compile_flags.hpp"

namespace mimir
{

/**
 * Interface class
 */
class IDynamicHeuristic
{
public:
    virtual ~IDynamicHeuristic() = default;

    [[nodiscard]] virtual double compute_heuristic(ConstView<StateDispatcher<StateReprTag>> state) = 0;
};

/**
 * Interface class
 */
template<typename Derived>
class IStaticHeuristic : public IDynamicHeuristic
{
private:
    using S = typename TypeTraits<Derived>::StateTag;
    using ConstStateView = ConstView<StateDispatcher<S>>;

    IStaticHeuristic() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

public:
    [[nodiscard]] double compute_heuristic(ConstStateView state) override { return self().compute_heuristic_impl(state); }
};

/**
 * ID base class.
 *
 * Derive from it to provide your own implementation.
 *
 * Define new template parameters to your derived tag
 * in the declaration file of your derived class.
 */
struct HeuristicTag
{
};

template<class DerivedTag>
concept IsHeuristicTag = std::derived_from<DerivedTag, HeuristicTag>;

/**
 * Dispatcher class.
 *
 * Wrap the tag to dispatch the correct overload.
 * The template parameters are arguments that all specializations have in common.
 * Do not add your specialized arguments here, add them to your derived tag instead.
 */
template<IsHeuristicTag H, IsStateTag S>
struct HeuristicDispatcher
{
};

template<typename T>
struct is_heuristic_dispatcher : std::false_type
{
};

template<IsHeuristicTag H, IsStateTag S>
struct is_heuristic_dispatcher<HeuristicDispatcher<H, S>> : std::true_type
{
};

template<typename T>
concept IsHeuristicDispatcher = is_heuristic_dispatcher<T>::value;

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