#ifndef MIMIR_SEARCH_HEURISTICS_BLIND_HPP_
#define MIMIR_SEARCH_HEURISTICS_BLIND_HPP_

#include "../heuristic_template.hpp"


namespace mimir
{

/**
 * ID class to dispatch a specialized implementation
*/
struct BlindTag : public HeuristicBaseTag { };


/**
 * Spezialized implementation class.
*/
template<IsPlanningModeTag P>
class Heuristic<WrappedHeuristicTag<BlindTag, P>> : public HeuristicBase<Heuristic<WrappedHeuristicTag<BlindTag,P>>> {
private:
    using PlanningModeTag = P;

    double compute_heuristic_impl(const View<State<P>>& state) {
        return 0.;
    }

    // Give access to the private interface implementations.
    template<typename>
    friend class HeuristicBase;

public:
    Heuristic(Problem problem) : HeuristicBase<Heuristic<WrappedHeuristicTag<BlindTag,P>>>(problem) { }
};


/**
 * Type traits.
*/
template<IsPlanningModeTag P>
struct TypeTraits<Heuristic<WrappedHeuristicTag<BlindTag,P>>> {
    using PlanningModeTag = P;
};



}  // namespace mimir

#endif  // MIMIR_SEARCH_HEURISTICS_ZERO_HPP_
