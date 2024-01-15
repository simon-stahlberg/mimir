#ifndef MIMIR_SEARCH_BRFS_HPP_
#define MIMIR_SEARCH_BRFS_HPP_

#include "search.hpp"


namespace mimir::search
{

/// @brief A general implementation of a breadth-first-search.
/// @tparam Configuration 
template<typename Configuration>
class BreadthFirstSearch : public SearchBase<BreadthFirstSearch<Configuration>> {
    // Implement configuration independent functionality. 
};

}  // namespace mimir::search

#endif  // MIMIR_SEARCH_BRFS_HPP_
