#ifndef MIMIR_SEARCH_ALGORITHMS_ASTAR_HPP_
#define MIMIR_SEARCH_ALGORITHMS_ASTAR_HPP_

#include "../algorithm_base.hpp"

#include "../heuristic.hpp"


namespace mimir
{

/**
 * Implementation class.
*/
template<typename C, template<typename> class H>
requires Config<C> && Heuristic<H<C>>
class AStar : public AlgorithmBase<AStar<C, H>> {
private:
    H<C> m_heuristic;

    SearchStatus find_solution_impl(GroundActionList& out_plan) {
        // TODO (Dominik): implement
        return SearchStatus::FAILED;
    }

    friend class AlgorithmBase<AStar<C, H>>;

public:
    AStar(const Problem& problem)
        : AlgorithmBase<AStar<C, H>>(problem)
        , m_heuristic(H<C>(problem)) { }
};


/**
 * Type traits.
*/
template<Config C, template<typename> class H>
struct TypeTraits<AStar<C, H>> {
    using ConfigType = C;
};



}  // namespace mimir

#endif  // MIMIR_SEARCH_ALGORITHMS_ASTAR_HPP_
