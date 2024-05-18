#ifndef MIMIR_SEARCH_AXIOM_EVALUATORS_STRATIFICATION_HPP_
#define MIMIR_SEARCH_AXIOM_EVALUATORS_STRATIFICATION_HPP_

#include "mimir/formalism/declarations.hpp"

namespace mimir
{

class AxiomPartition
{
private:
    // All axioms
    AxiomSet m_axioms;
    // Axioms without derived literal in the body.
    AxiomSet m_initially_relevant_axioms;

    std::unordered_map<Predicate, AxiomSet> m_axioms_by_body_predicates;

public:
    explicit AxiomPartition(AxiomSet axioms, const PredicateSet& derived_predicates, const PredicateSet& affected_derived_predicates);

    /// @brief Retrieve all axioms that have an atom with the same predicate in the body.
    void retrieve_axioms_with_same_body_predicate(GroundAtom derived_atom, AxiomSet& ref_axioms) const;

    const AxiomSet& get_axioms() const;
    const AxiomSet& get_initially_relevant_axioms() const;
};

/// @brief Compute axiom partitioning using stratification.
/// An implementation of Algorithm 1 by Thiébaux-et-al-ijcai2003
/// Source: https://users.cecs.anu.edu.au/~thiebaux/papers/ijcai03.pdf
/// @param axioms
/// @return
extern std::vector<AxiomPartition> compute_axiom_partitioning(const AxiomList& axioms, const PredicateList& derived_predicates);

}

#endif
