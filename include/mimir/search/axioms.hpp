#ifndef MIMIR_AXIOMS_HPP_
#define MIMIR_AXIOMS_HPP_

#include <vector>

/**
 * Include all specializations here
 */
#if defined(STATE_REPR_DENSE)

#include "mimir/search/axioms/dense.hpp"

namespace mimir
{
using GroundAxiomBuilder = DenseGroundAxiomBuilder;
using GroundAxiom = DenseGroundAxiom;
using GroundAxiomList = std::vector<GroundAxiom>;
}

#else
#error "State representation undefined."
#endif

#endif  // MIMIR_AXIOMS_HPP_
