/*
 * Copyright (C) 2023 Dominik Drexler and Simon Stahlberg
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef MIMIR_SEARCH_AXIOM_EVALUATORS_STRATIFICATION_HPP_
#define MIMIR_SEARCH_AXIOM_EVALUATORS_STRATIFICATION_HPP_

#include "mimir/formalism/formalism.hpp"

namespace mimir
{

class AxiomPartition
{
private:
    // All axioms
    AxiomSet m_axioms;
    // Axioms without derived literal in the body.
    AxiomSet m_initially_relevant_axioms;

    std::unordered_map<Predicate<Derived>, AxiomSet> m_axioms_by_body_predicates;

public:
    explicit AxiomPartition(AxiomSet axioms, const PredicateSet<Derived>& derived_predicates, const PredicateSet<Derived>& affected_derived_predicates);

    /// @brief Retrieve all axioms that have an atom with the same predicate in the body.
    void retrieve_axioms_with_same_body_predicate(GroundAtom<Derived> derived_atom, AxiomSet& ref_axioms) const;

    const AxiomSet& get_axioms() const;
    const AxiomSet& get_initially_relevant_axioms() const;
};

/// @brief Compute axiom partitioning using stratification.
/// An implementation of Algorithm 1 by Thiébaux-et-al-ijcai2003
/// Source: https://users.cecs.anu.edu.au/~thiebaux/papers/ijcai03.pdf
/// @param axioms
/// @return
extern std::vector<AxiomPartition> compute_axiom_partitioning(const AxiomList& axioms, const PredicateList<Derived>& derived_predicates);

}

#endif
