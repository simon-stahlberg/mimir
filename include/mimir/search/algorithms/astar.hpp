#ifndef MIMIR_SEARCH_ALGORITHMS_ASTAR_HPP_
#define MIMIR_SEARCH_ALGORITHMS_ASTAR_HPP_

#include "../algorithm_template.hpp"

#include "../heuristics.hpp"


namespace mimir
{

/**
 * ID class to dispatch a specialized implementation
*/
template<IsPlanningMode P, IsHeuristic H>
requires HaveEqualPlanningMode<P, H>
struct AStar : public AlgorithmBaseTag { };


/**
 * Spezialized implementation class.
*/
template<IsPlanningMode P, IsHeuristic H>
requires HaveEqualPlanningMode<P, H>
class Algorithm<AStar<P, H>> : public AlgorithmBase<Algorithm<AStar<P, H>>> {
private:
    Heuristic<H> m_heuristic;

    SearchStatus find_solution_impl(GroundActionList& out_plan) {
        // TODO (Dominik): implement
        return SearchStatus::FAILED;
    }

    friend class AlgorithmBase<Algorithm<AStar<P, H>>>;

public:
    Algorithm(const Problem& problem)
        : AlgorithmBase<Algorithm<AStar<P, H>>>(problem)
        , m_heuristic(problem) { }
};


/**
 * Type traits.
*/
template<IsPlanningMode P, IsHeuristic H>
struct TypeTraits<AStar<P, H>> {
    using PlanningMode = P;
};

template<IsPlanningMode P, IsHeuristic H>
struct TypeTraits<Algorithm<AStar<P, H>>> {
    using PlanningMode = P;
};



}  // namespace mimir

#endif  // MIMIR_SEARCH_ALGORITHMS_ASTAR_HPP_
