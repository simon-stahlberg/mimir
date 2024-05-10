#ifndef MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_HPP_
#define MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_HPP_

/**
 * Include all specializations here
 */

#if defined(STATE_REPR_DENSE)

#include "mimir/search/applicable_action_generators/dense_grounded.hpp"
#include "mimir/search/applicable_action_generators/dense_lifted.hpp"

namespace mimir
{
using LiftedAAG = AAG<LiftedAAGDispatcher<DenseStateTag>>;
using GroundedAAG = AAG<GroundedAAGDispatcher<DenseStateTag>>;
}

#else
#error "State representation undefined."
#endif

#endif  // MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_HPP_
