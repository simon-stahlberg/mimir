#ifndef MIMIR_SEARCH_STATE_HPP_
#define MIMIR_SEARCH_STATE_HPP_

#include "config.hpp"

#include <cstdint>


namespace mimir
{

/**
 * ID class
*/
template<Config C> class State { };

/**
 * Data types
*/
using state_id_type = uint32_t;

}  // namespace mimir

#endif  // MIMIR_SEARCH_STATE_HPP_
