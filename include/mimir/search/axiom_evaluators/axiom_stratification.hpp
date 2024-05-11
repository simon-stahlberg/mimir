#ifndef MIMIR_SEARCH_AXIOM_EVALUATORS_STRATIFICATION_HPP_
#define MIMIR_SEARCH_AXIOM_EVALUATORS_STRATIFICATION_HPP_

#include "mimir/formalism/declarations.hpp"

namespace mimir
{

class AxiomPartition
{
private:
    AxiomSet m_axioms;

    std::unordered_map<Predicate, AxiomSet> m_axioms_by_body_predicates;

public:
    explicit AxiomPartition(AxiomSet axioms);

    /// @brief Retrieve all axioms that have an atom with the same predicate in the body.
    void on_generate_derived_ground_atom(GroundAtom derived_atom, AxiomSet& ref_axioms) const;

    const AxiomSet& get_axioms() const;
};

/// @brief Compute axiom partitioning using stratification.
/// An implementation of Algorithm 1 by Thiébaux-et-al-ijcai2003
/// Source: https://users.cecs.anu.edu.au/~thiebaux/papers/ijcai03.pdf
/// @param axioms
/// @return
extern std::vector<AxiomPartition> compute_axiom_partitioning(const AxiomList& axioms, const PredicateList& derived_predicates);

}

#endif
