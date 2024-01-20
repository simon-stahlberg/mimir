#ifndef MIMIR_SEARCH_SEARCH_NODE_HPP_
#define MIMIR_SEARCH_SEARCH_NODE_HPP_

#include "config.hpp"


namespace mimir {

/**
 * ID class
*/
template<Config C> class SearchNode { };


/**
 * Data types
*/
enum SearchNodeStatus {NEW = 0, OPEN = 1, CLOSED = 2, DEAD_END = 3};

using g_value_type = int;


} // namespace mimir



#endif  // MIMIR_SEARCH_SEARCH_NODE_HPP_
