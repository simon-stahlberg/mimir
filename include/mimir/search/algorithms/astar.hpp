#ifndef MIMIR_SEARCH_ALGORITHMS_ASTAR_HPP_
#define MIMIR_SEARCH_ALGORITHMS_ASTAR_HPP_

#include "../algorithm_base.hpp"


namespace mimir
{

/// @brief A general implementation of AStar.
/// @tparam Configuration
template<typename Configuration, template<typename> class Heuristic>
class AStar : public AlgorithmBase<AStar<Configuration, Heuristic>> {
private:
    Heuristic<Configuration> m_heuristic;

    // Implement configuration independent functionality.

    void find_solution_impl() {
        // TODO (Dominik): implement
    }

    friend class AlgorithmBase<AStar<Configuration, Heuristic>>;
};


template<typename Configuration, template<typename> class Heuristic>
struct TypeTraits<AStar<Configuration, Heuristic>> {
    using ConfigurationType = Configuration;
};



}  // namespace mimir

#endif  // MIMIR_SEARCH_ALGORITHMS_ASTAR_HPP_
