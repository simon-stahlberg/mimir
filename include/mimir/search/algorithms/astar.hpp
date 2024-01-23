#ifndef MIMIR_SEARCH_ALGORITHMS_ASTAR_HPP_
#define MIMIR_SEARCH_ALGORITHMS_ASTAR_HPP_

#include "../algorithm_template.hpp"

#include "../heuristics.hpp"


namespace mimir
{

/**
 * ID class to dispatch a specialized implementation
*/
template<IsPlanningModeTag P, IsHeuristicTag H>
struct AStar : public AlgorithmBaseTag { };


/**
 * Spezialized implementation class.
*/
template<IsPlanningModeTag P, IsHeuristicTag H, IsApplicableActionGeneratorTag AG>
class Algorithm<AStar<P, H>, AG> : public AlgorithmBase<Algorithm<AStar<P, H>, AG>, AG> {
private:
    Heuristic<HeuristicInstantiation<H, P>> m_heuristic;

    SearchStatus find_solution_impl(GroundActionList& out_plan) {
        // TODO (Dominik): implement
        return SearchStatus::FAILED;
    }

    // Correct friend declaration
    friend class AlgorithmBase<Algorithm<AStar<P, H>, AG>, AG>;

public:
    Algorithm(const Problem& problem)
        : AlgorithmBase<Algorithm<AStar<P, H>, AG>, AG>(problem)
        , m_heuristic(problem) { }
};


/**
 * Type traits.
*/
template<IsPlanningModeTag P, IsHeuristicTag H, IsApplicableActionGeneratorTag AG>
struct TypeTraits<Algorithm<AStar<P, H>, AG>> {
    using PlanningMode = P;
};


}  // namespace mimir

#endif  // MIMIR_SEARCH_ALGORITHMS_ASTAR_HPP_
