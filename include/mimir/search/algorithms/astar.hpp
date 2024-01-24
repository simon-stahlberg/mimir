#ifndef MIMIR_SEARCH_ALGORITHMS_ASTAR_HPP_
#define MIMIR_SEARCH_ALGORITHMS_ASTAR_HPP_

#include "template.hpp"

#include "../heuristics.hpp"


namespace mimir
{

/**
 * ID class to dispatch a specialized implementation
*/
template<IsPlanningModeTag P
       , IsHeuristicTag H
       , IsStateTag S = BitsetStateTag
       , IsActionTag A = DefaultActionTag
       , IsAAGTag AG = DefaultAAGTag
       , IsSSGTag SG = DefaultSSGTag>
struct AStarTag : public AlgorithmBaseTag { };


/**
 * Spezialized implementation class.
*/
template<IsPlanningModeTag P, IsHeuristicTag H, IsStateTag S, IsActionTag A, IsAAGTag AG, IsSSGTag SG>
class Algorithm<AStarTag<P, H, S, A, AG, SG>> : public AlgorithmBase<Algorithm<AStarTag<P, H, S, A, AG, SG>>> {
private:
    using StateView = View<WrappedStateTag<S, P>>;
    using ActionView = View<WrappedActionTag<A, P, S>>;
    using ActionViewList = std::vector<ActionView>;

    Heuristic<WrappedHeuristicTag<H, P, S>> m_heuristic;

    SearchStatus find_solution_impl(ActionViewList& out_plan) {
        // TODO (Dominik): implement
        return SearchStatus::FAILED;
    }

    // Give access to the private interface implementations.
    template<typename>
    friend class AlgorithmBase;

public:
    Algorithm(const Problem& problem)
        : AlgorithmBase<Algorithm<AStarTag<P, H, S, A, AG, SG>>>(problem)
        , m_heuristic(problem) { }
};


/**
 * Type traits.
*/
template<IsPlanningModeTag P, IsHeuristicTag H, IsStateTag S, IsActionTag A, IsAAGTag AG, IsSSGTag SG>
struct TypeTraits<Algorithm<AStarTag<P, H, S, A, AG, SG>>> {
    using PlanningModeTag = P;
    using HeuristicTag = H;
    using StateTag = S;
    using ActionTag = A;
    using AAGTag = AG;
    using SSGTag = SG;

    using ActionView = View<WrappedActionTag<A, P, S>>;
    using ActionViewList = std::vector<ActionView>;
};


}  // namespace mimir

#endif  // MIMIR_SEARCH_ALGORITHMS_ASTAR_HPP_
