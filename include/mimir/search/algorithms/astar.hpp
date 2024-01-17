#ifndef MIMIR_SEARCH_ALGORITHMS_ASTAR_HPP_
#define MIMIR_SEARCH_ALGORITHMS_ASTAR_HPP_

#include "../algorithm_base.hpp"


namespace mimir
{

/// @brief A general implementation of AStar.
/// @tparam Config
template<typename Config, template<typename> typename Heuristic>
class AStar : public AlgorithmBase<AStar<Config, Heuristic>> {
    // Implement configuration independent functionality.
private:
    Heuristic<Config> m_heuristic;

    void find_solution_impl() {
        // TODO (Dominik): implement
    }

    friend class AlgorithmBase<AStar<Config, Heuristic>>;

public:
    AStar(const Problem* problem) 
        : AlgorithmBase<AStar<Config, Heuristic>>(problem) { }
};


template<typename Config, template<typename> typename Heuristic>
struct TypeTraits<AStar<Config, Heuristic>> {
    using ConfigType = Config;
};



}  // namespace mimir

#endif  // MIMIR_SEARCH_ALGORITHMS_ASTAR_HPP_
