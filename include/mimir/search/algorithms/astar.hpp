#ifndef MIMIR_SEARCH_ALGORITHMS_ASTAR_HPP_
#define MIMIR_SEARCH_ALGORITHMS_ASTAR_HPP_

#include "../algorithm_template.hpp"

#include "../heuristics.hpp"


namespace mimir
{

/**
 * ID class to dispatch a specialized implementation
*/
template<IsPlanningModeTag P, IsHeuristicTag H, IsApplicableActionGeneratorTag AG = DefaultApplicableActionGeneratorTag, IsSuccessorStateGeneratorTag SG = DefaultSuccessorStateGeneratorTag>
struct AStarTag : public AlgorithmBaseTag { };


/**
 * Spezialized implementation class.
*/
template<IsPlanningModeTag P, IsHeuristicTag H, IsApplicableActionGeneratorTag AG, IsSuccessorStateGeneratorTag SG>
class Algorithm<AStarTag<P, H, AG, SG>> : public AlgorithmBase<Algorithm<AStarTag<P, H, AG, SG>>> {
private:
    Heuristic<HeuristicInstantiation<H, P>> m_heuristic;

    SearchStatus find_solution_impl(GroundActionList& out_plan) {
        // TODO (Dominik): implement
        return SearchStatus::FAILED;
    }

    // Correct friend declaration
    friend class AlgorithmBase<Algorithm<AStarTag<P, H, AG, SG>>>;

public:
    Algorithm(const Problem& problem)
        : AlgorithmBase<Algorithm<AStarTag<P, H, AG, SG>>>(problem)
        , m_heuristic(problem) { }
};


/**
 * Type traits.
*/
template<IsPlanningModeTag P, IsHeuristicTag H, IsApplicableActionGeneratorTag AG, IsSuccessorStateGeneratorTag SG>
struct TypeTraits<Algorithm<AStarTag<P, H, AG, SG>>> {
    using PlanningModeTag = P;
    using HeuristicTag = H;
    using ApplicableActionGeneratorTag = AG;
    using SuccessorStateGeneratorTag = SG;
};


}  // namespace mimir

#endif  // MIMIR_SEARCH_ALGORITHMS_ASTAR_HPP_
