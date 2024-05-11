#ifndef MIMIR_SEARCH_AXIOM_EVALUATORS_STRATIFICATION_HPP_
#define MIMIR_SEARCH_AXIOM_EVALUATORS_STRATIFICATION_HPP_

#include "mimir/formalism/declarations.hpp"

namespace mimir
{

/// @brief Compute axiom partitioning using stratification.
/// An implementation of Algorithm 1 by Thiébaux-et-al-ijcai2003
/// Source: https://users.cecs.anu.edu.au/~thiebaux/papers/ijcai03.pdf
/// @param axioms
/// @return
extern std::vector<AxiomList> compute_axiom_partitioning(const AxiomList& axioms, const PredicateList& derived_predicates);

}

#endif
