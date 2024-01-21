#ifndef MIMIR_SEARCH_ALGORITHMS_ASTAR_HPP_
#define MIMIR_SEARCH_ALGORITHMS_ASTAR_HPP_

#include "../algorithm_base.hpp"

#include "../heuristic.hpp"


namespace mimir
{

/**
 * Implementation class.
*/
template<typename Config, template<typename> typename HeuristicTag>
class AStar : public AlgorithmBase<AStar<Config, HeuristicTag>> {
    // Implement configuration independent functionality.
private:
    Heuristic<HeuristicTag<Config>> m_heuristic;

    SearchStatus find_solution_impl(GroundActionList& out_plan) {
        // TODO (Dominik): implement
        return SearchStatus::FAILED;
    }

    friend class AlgorithmBase<AStar<Config, HeuristicTag>>;

public:
    AStar(const Problem& problem)
        : AlgorithmBase<AStar<Config, HeuristicTag>>(problem)
        , m_heuristic(Heuristic<HeuristicTag<Config>>(problem)) { }
};


/**
 * Type traits.
*/
template<Config C, template<typename> typename HeuristicTag>
struct TypeTraits<AStar<C, HeuristicTag>> {
    using ConfigTag = C;
};



}  // namespace mimir

#endif  // MIMIR_SEARCH_ALGORITHMS_ASTAR_HPP_
