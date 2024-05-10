#ifndef MIMIR_SEARCH_STATES_HPP_
#define MIMIR_SEARCH_STATES_HPP_

#include <vector>

/**
 * Include all specializations here
 */

#if defined(STATE_REPR_DENSE)

#include "mimir/search/states/dense.hpp"

namespace mimir
{
using State = ConstView<StateDispatcher<DenseStateTag>>;
using StateList = std::vector<State>;
}

#else
#error "State representation undefined."
#endif

#endif  // MIMIR_SEARCH_STATES_HPP_
