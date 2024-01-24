#ifndef MIMIR_SEARCH_HEURISTICS_TEMPLATE_HPP_
#define MIMIR_SEARCH_HEURISTICS_TEMPLATE_HPP_

#include "../grounded/state_view.hpp"
#include "../lifted/state_view.hpp"
#include "../type_traits.hpp"

#include "../../common/mixins.hpp"
#include "../../formalism/problem/declarations.hpp"


namespace mimir
{

/**
 * Interface class
*/
template<typename Derived>
class HeuristicBase : public UncopyableMixin<HeuristicBase<Derived>> {
private:
    using P = typename TypeTraits<Derived>::PlanningModeTag;

    HeuristicBase(Problem problem) : m_problem(problem) { }
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

    Problem m_problem;

public:
    [[nodiscard]] double compute_heuristic(View<State<P>> state) {
        return self().compute_heuristic_impl(state);
    }
};


/**
 * ID class.
 *
 * Derive from it to provide your own implementation of a heuristic.
*/
struct HeuristicBaseTag {};

template<class DerivedTag>
concept IsHeuristicTag = std::derived_from<DerivedTag, HeuristicBaseTag>;


/**
 * Wrapper class.
 *
 * Wrap the tag and the planning mode to be able use a given planning mode.
*/
template<IsHeuristicTag H, IsPlanningModeTag P>
struct WrappedHeuristicTag {
    using HeuristicTag = H;
    using PlanningModeTag = P;
};

template<typename T>
concept IsWrappedHeuristicTag = requires {
    typename T::HeuristicTag;
    typename T::PlanningModeTag;
};


/**
 * General implementation class.
 *
 * Spezialize the wrapped tag to provide your own implementation of a heuristic.
*/
template<IsWrappedHeuristicTag T>
class Heuristic : public HeuristicBase<Heuristic<T>> { };




}  // namespace mimir

#endif  // MIMIR_SEARCH_HEURISTIC_TEMPLATE_HPP_
