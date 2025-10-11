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

#include "mimir/formalism/axiom_partitioning.hpp"

#include "mimir/formalism/atom.hpp"
#include "mimir/formalism/axiom.hpp"
#include "mimir/formalism/conjunctive_condition.hpp"
#include "mimir/formalism/ground_atom.hpp"
#include "mimir/formalism/literal.hpp"

#include <algorithm>

namespace mimir::formalism
{

AxiomPartition::AxiomPartition(AxiomSet axioms,
                               const PredicateSet<DerivedTag>& derived_predicates,
                               const PredicateSet<DerivedTag>& affected_derived_predicates_in_earlier_partition) :
    m_axioms(std::move(axioms)),
    m_initially_relevant_axioms(),
    m_axioms_by_body_predicates()
{
    for (const auto& axiom : m_axioms)
    {
        bool is_relevant_first = true;

        for (const auto& literal : axiom->get_conjunctive_condition()->get_literals<DerivedTag>())
        {
            const auto& predicate = literal->get_atom()->get_predicate();

            m_axioms_by_body_predicates[predicate].insert(axiom);

            if (derived_predicates.count(predicate) && !affected_derived_predicates_in_earlier_partition.count(predicate))
            {
                // axioms whose derived body literals were not affected in an earlier partition are not relevant initially.
                // because the precondition cannot be satisfied initially.
                is_relevant_first = false;
            }
        }

        if (is_relevant_first)
        {
            m_initially_relevant_axioms.insert(axiom);
        }
    }
}

void AxiomPartition::retrieve_axioms_with_same_body_predicate(GroundAtom<DerivedTag> derived_atom, AxiomSet& ref_axioms) const
{
    auto it = m_axioms_by_body_predicates.find(derived_atom->get_predicate());
    if (it != m_axioms_by_body_predicates.end())
    {
        const auto& relevant_axioms = it->second;

        ref_axioms.insert(relevant_axioms.begin(), relevant_axioms.end());
    }
}

const AxiomSet& AxiomPartition::get_axioms() const { return m_axioms; }

const AxiomSet& AxiomPartition::get_initially_relevant_axioms() const { return m_initially_relevant_axioms; }

}