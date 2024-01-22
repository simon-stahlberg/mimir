#ifndef MIMIR_SEARCH_SEARCH_NODE_TAG_HPP_
#define MIMIR_SEARCH_SEARCH_NODE_TAG_HPP_

#include "config.hpp"


namespace mimir {

/**
 * ID class
*/
template<typename C>
requires IsConfig<C>
class SearchNode { };


} // namespace mimir



#endif  // MIMIR_SEARCH_SEARCH_NODE_TAG_HPP_
