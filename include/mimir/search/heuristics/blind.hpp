#ifndef MIMIR_SEARCH_HEURISTICS_BLIND_HPP_
#define MIMIR_SEARCH_HEURISTICS_BLIND_HPP_

#include "template.hpp"


namespace mimir
{

/**
 * ID class to dispatch a specialized implementation
*/
struct BlindTag : public HeuristicBaseTag { };


/**
 * Spezialized implementation class.
*/
template<IsPlanningModeTag P, IsStateTag S>
class Heuristic<WrappedHeuristicTag<BlindTag, P, S>>
    : public HeuristicBase<Heuristic<WrappedHeuristicTag<BlindTag, P, S>>> {
private:
    using StateView = View<WrappedStateTag<S, P>>;

    double compute_heuristic_impl(const StateView& state) {
        return 0.;
    }

    // Give access to the private interface implementations.
    template<typename>
    friend class HeuristicBase;

public:
    Heuristic(Problem problem) : HeuristicBase<Heuristic<WrappedHeuristicTag<BlindTag, P, S>>>(problem) { }
};


/**
 * Type traits.
*/
template<IsPlanningModeTag P, IsStateTag S>
struct TypeTraits<Heuristic<WrappedHeuristicTag<BlindTag, P, S>>> {
    using PlanningModeTag = P;
    using StateTag = S;
};



}  // namespace mimir

#endif  // MIMIR_SEARCH_HEURISTICS_BLIND_HPP_
