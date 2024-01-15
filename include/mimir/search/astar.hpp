#ifndef MIMIR_SEARCH_ASTAR_HPP_
#define MIMIR_SEARCH_ASTAR_HPP_

#include "search.hpp"


namespace mimir::search
{

/// @brief A general implementation of AStar.
/// @tparam Configuration 
template<typename Configuration>
class AStar : public SearchBase<AStar<Configuration>> {
    // Implement configuration independent functionality. 
};



}  // namespace mimir::search

#endif  // MIMIR_FORMALISM_SEARCH_HPP_
