#ifndef MIMIR_SEARCH_ALGORITHMS_ASTAR_HPP_
#define MIMIR_SEARCH_ALGORITHMS_ASTAR_HPP_

#include "../search_base.hpp"


namespace mimir::search
{

/// @brief A general implementation of AStar.
/// @tparam Configuration
template<typename Configuration>
class AStar : public SearchBase<AStar<Configuration>> {
private:
    // Implement configuration independent functionality.
};



}  // namespace mimir::search

#endif  // MIMIR_SEARCH_ALGORITHMS_ASTAR_HPP_
