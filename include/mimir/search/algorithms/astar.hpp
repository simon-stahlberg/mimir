#ifndef MIMIR_SEARCH_ALGORITHMS_ASTAR_HPP_
#define MIMIR_SEARCH_ALGORITHMS_ASTAR_HPP_

#include "../algorithm_template.hpp"

#include "../heuristics.hpp"


namespace mimir
{

/**
 * ID class to dispatch a specialized implementation
*/
template<IsHeuristicTag H>
struct AStarTag : public AlgorithmBaseTag { };


/**
 * Spezialized implementation class.
*/
template<IsPlanningModeTag P, IsHeuristicTag H, IsApplicableActionGeneratorTag AG>
class Algorithm<AStarTag<H>, P, AG> : public AlgorithmBase<Algorithm<AStarTag<H>, P, AG>, P, AG> {
private:
    Heuristic<HeuristicInstantiation<H, P>, P> m_heuristic;

    SearchStatus find_solution_impl(GroundActionList& out_plan) {
        // TODO (Dominik): implement
        return SearchStatus::FAILED;
    }

    // Correct friend declaration
    friend class AlgorithmBase<Algorithm<AStarTag<H>, P, AG>, P, AG>;

public:
    Algorithm(const Problem& problem)
        : AlgorithmBase<Algorithm<AStarTag<H>, P, AG>, P, AG>(problem)
        , m_heuristic(problem) { }
};


}  // namespace mimir

#endif  // MIMIR_SEARCH_ALGORITHMS_ASTAR_HPP_
