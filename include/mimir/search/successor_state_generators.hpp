#ifndef MIMIR_SEARCH_SUCCESSOR_STATE_GENERATORS_HPP_
#define MIMIR_SEARCH_SUCCESSOR_STATE_GENERATORS_HPP_

/**
 * Include all specializations here
 */

#if defined(STATE_REPR_DENSE)

#include "mimir/search/successor_state_generators/dense.hpp"

namespace mimir
{
using SuccessorStateGenerator = DenseSSG;
}

#else
#error "State representation undefined."
#endif

#endif  // MIMIR_SEARCH_SUCCESSOR_STATE_GENERATORS_HPP_
