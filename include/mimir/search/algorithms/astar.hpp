#ifndef MIMIR_SEARCH_ALGORITHMS_ASTAR_HPP_
#define MIMIR_SEARCH_ALGORITHMS_ASTAR_HPP_

#include "../algorithm_base.hpp"

#include "../heuristic.hpp"


namespace mimir
{

/**
 * ID class.
*/
template<typename C, typename H>
requires IsConfig<C> && IsHeuristic<H>
struct AStar { };


/**
 * Spezialized implementation class.
*/
template<typename C, typename H>
class Algorithm<AStar<C, H>> : public AlgorithmBase<Algorithm<AStar<C, H>>> {
private:
    H m_heuristic;

    SearchStatus find_solution_impl(GroundActionList& out_plan) {
        // TODO (Dominik): implement
        return SearchStatus::FAILED;
    }

    friend class AlgorithmBase<Algorithm<AStar<C, H>>>;

public:
    Algorithm(const Problem& problem)
        : AlgorithmBase<Algorithm<AStar<C, H>>>(problem)
        , m_heuristic(problem) { }
};


/**
 * Type traits.
*/
template<typename C, typename H>
requires IsConfig<C>
struct TypeTraits<Algorithm<AStar<C, H>>> {
    using Config = C;
};



}  // namespace mimir

#endif  // MIMIR_SEARCH_ALGORITHMS_ASTAR_HPP_
