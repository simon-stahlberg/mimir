#ifndef MIMIR_SEARCH_AXIOM_EVALUATORS_HPP_
#define MIMIR_SEARCH_AXIOM_EVALUATORS_HPP_

/**
 * Include all specializations here
 */

#if defined(STATE_REPR_DENSE)

#include "mimir/search/axiom_evaluators/dense.hpp"

namespace mimir
{
using AxiomEvaluator = AE<AEDispatcher<DenseStateTag>>;
}

#else
#error "State representation undefined."
#endif

#endif  // MIMIR_SEARCH_AXIOM_EVALUATORS_HPP_
