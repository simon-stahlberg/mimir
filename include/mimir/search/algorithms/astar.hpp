#ifndef MIMIR_SEARCH_ALGORITHMS_ASTAR_HPP_
#define MIMIR_SEARCH_ALGORITHMS_ASTAR_HPP_

#include "../algorithm_base.hpp"

#include "../heuristic.hpp"


namespace mimir
{

/**
 * Spezialized implementation class.
*/
template<typename C, typename H>
class AStarAlgorithm : public AlgorithmBase<AStarAlgorithm<C, H>> {
private:
    H m_heuristic;

    SearchStatus find_solution_impl(GroundActionList& out_plan) {
        // TODO (Dominik): implement
        return SearchStatus::FAILED;
    }

    friend class AlgorithmBase<AStarAlgorithm<C, H>>;

public:
    AStarAlgorithm(const Problem& problem)
        : AlgorithmBase<AStarAlgorithm<C, H>>(problem)
        , m_heuristic(problem) { }
};


/**
 * Type traits.
*/
template<typename C, typename H>
requires IsConfig<C>
struct TypeTraits<AStarAlgorithm<C, H>> {
    using Config = C;
    using Heuristic = H;
};



}  // namespace mimir

#endif  // MIMIR_SEARCH_ALGORITHMS_ASTAR_HPP_
