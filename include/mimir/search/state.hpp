#ifndef MIMIR_SEARCH_STATE_TAG_HPP_
#define MIMIR_SEARCH_STATE_TAG_HPP_

#include "config.hpp"

#include <cstdint>


namespace mimir
{

/**
 * ID class
*/
template<typename C> 
requires IsConfig<C>
class State { };

}  // namespace mimir

#endif  // MIMIR_SEARCH_STATE_HPP_
