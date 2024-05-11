#ifndef MIMIR_SEARCH_HEURISTICS_HPP_
#define MIMIR_SEARCH_HEURISTICS_HPP_

/**
 * Include all specializations here
 */

// can use compile flags to define different representations
#if defined(STATE_REPR_DENSE)

#include "mimir/search/heuristics/blind.hpp"

namespace mimir
{
using BlindHeuristic = BlindDenseHeuristic;
}

#else
#error "State representation undefined."
#endif

#endif  // MIMIR_SEARCH_HEURISTICS_HPP_
