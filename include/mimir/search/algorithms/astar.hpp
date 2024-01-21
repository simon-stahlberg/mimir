#ifndef MIMIR_SEARCH_ALGORITHMS_ASTAR_HPP_
#define MIMIR_SEARCH_ALGORITHMS_ASTAR_HPP_

#include "../algorithm_base.hpp"

#include "../heuristic.hpp"


namespace mimir
{

/**
 * ID class.
*/
template<Config C, template<typename> typename HeuristicTag>
struct AStarAlgorithmTag {
    //using H = Heuristic<HeuristicTag<C>>;
};


/**
 * Implementation class.
*/
template<Config C, template<typename> typename HeuristicTag>
class Algorithm<AStarAlgorithmTag<C, HeuristicTag>> : public AlgorithmBase<Algorithm<AStarAlgorithmTag<C, HeuristicTag>>> {
    // Implement configuration independent functionality.
private:
    Heuristic<HeuristicTag<C>> m_heuristic;

    SearchStatus find_solution_impl(GroundActionList& out_plan) {
        // TODO (Dominik): implement
        return SearchStatus::FAILED;
    }

    friend class AlgorithmBase<Algorithm<AStarAlgorithmTag<C, HeuristicTag>>>;

public:
    Algorithm(const Problem& problem)
        : AlgorithmBase<Algorithm<AStarAlgorithmTag<C, HeuristicTag>>>(problem)
        , m_heuristic(Heuristic<HeuristicTag<C>>(problem)) { }
};


/**
 * Type traits.
*/
template<Config C, template<typename> typename HeuristicTag>
struct TypeTraits<Algorithm<AStarAlgorithmTag<C, HeuristicTag>>> {
    using ConfigTag = C;
};



}  // namespace mimir

#endif  // MIMIR_SEARCH_ALGORITHMS_ASTAR_HPP_
