#ifndef MIMIR_SEARCH_ALGORITHMS_ASTAR_HPP_
#define MIMIR_SEARCH_ALGORITHMS_ASTAR_HPP_

#include "../algorithm_base.hpp"

#include "../heuristic.hpp"


namespace mimir
{

/// @brief A general implementation of AStar.
/// @tparam Config
template<typename Config, template<typename> typename HeuristicID>
class AStar : public AlgorithmBase<AStar<Config, HeuristicID>> {
    // Implement configuration independent functionality.
private:
    Heuristic<HeuristicID<Config>> m_heuristic;

    SearchStatus find_solution_impl(GroundActionList& out_plan) {
        // TODO (Dominik): implement
        return SearchStatus::FAILED;
    }

    friend class AlgorithmBase<AStar<Config, HeuristicID>>;

public:
    AStar(const Problem& problem)
        : AlgorithmBase<AStar<Config, HeuristicID>>(problem) {
        }
};


template<typename Config, template<typename> typename HeuristicID>
struct TypeTraits<AStar<Config, HeuristicID>> {
    using ConfigType = Config;
};



}  // namespace mimir

#endif  // MIMIR_SEARCH_ALGORITHMS_ASTAR_HPP_
