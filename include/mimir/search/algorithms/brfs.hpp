#ifndef MIMIR_SEARCH_ALGORITHMS_BRFS_HPP_
#define MIMIR_SEARCH_ALGORITHMS_BRFS_HPP_

#include "../algorithm_base.hpp"


namespace mimir
{

/// @brief A general implementation of a breadth-first-search.
/// @tparam Configuration
template<typename Configuration>
class BreadthFirstSearch : public AlgorithmBase<BreadthFirstSearch<Configuration>> {
private:
    // Implement configuration independent functionality.
};

}  // namespace mimir

#endif  // MIMIR_SEARCH_ALGORITHMS_BRFS_HPP_
