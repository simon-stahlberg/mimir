#ifndef MIMIR_SEARCH_HEURISTICS_BLIND_HPP_
#define MIMIR_SEARCH_HEURISTICS_BLIND_HPP_

#include "../heuristic.hpp"


namespace mimir
{

/**
 * ID class to dispatch a specialized implementation
*/
template<IsPlanningMode P>
struct Blind : public HeuristicBaseTag { };


/**
 * Spezialized implementation class.
*/
template<IsPlanningMode P>
class Heuristic<Blind<P>> : public HeuristicBase<Heuristic<Blind<P>>> {
private:
    double compute_heuristic_impl(const View<State<P>>& state) {
        return 0.;
    }

    friend class HeuristicBase<Heuristic<Blind<P>>>;

public:
    Heuristic(Problem problem) : HeuristicBase<Heuristic<Blind<P>>>(problem) { }
};


/**
 * Type traits.
*/
template<IsPlanningMode P>
struct TypeTraits<Heuristic<Blind<P>>> {
    using Tag = Blind<P>;
    using PlanningMode = P;
};

template<IsPlanningMode P>
struct TypeTraits<Blind<P>> {
    using PlanningMode = P;
};



}  // namespace mimir

#endif  // MIMIR_SEARCH_HEURISTICS_ZERO_HPP_
