#ifndef MIMIR_SEARCH_SEARCH_NODE_HPP_
#define MIMIR_SEARCH_SEARCH_NODE_HPP_

#include "config.hpp"


namespace mimir {

/**
 * Enum classes
*/
enum SearchNodeStatus {NEW = 0, OPEN = 1, CLOSED = 2, DEAD_END = 3};


/**
 * ID class
*/
template<Config C> class SearchNode { };


} // namespace mimir



#endif  // MIMIR_SEARCH_SEARCH_NODE_HPP_
