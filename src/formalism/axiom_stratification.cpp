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

#include "mimir/formalism/axiom_stratification.hpp"

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

/// @brief Compute occurrences of predicates in axiom heads
/// @param axioms a set of axioms
/// @param predicates the set of all predicates occuring in axioms
static std::unordered_map<Predicate<DerivedTag>, AxiomList> compute_predicate_head_occurrences(const AxiomList& axioms)
{
    auto predicate_occurrences = std::unordered_map<Predicate<DerivedTag>, AxiomList> {};
    for (const auto& axiom : axioms)
    {
        predicate_occurrences[axiom->get_literal()->get_atom()->get_predicate()].push_back(axiom);
    }
    return predicate_occurrences;
}

enum class StratumStatus
{
    UNCONSTRAINED = 0,
    LOWER = 1,
    STRICTLY_LOWER = 2,
};

static std::vector<PredicateSet<DerivedTag>> compute_stratification(const AxiomList& axioms, const PredicateList<DerivedTag>& derived_predicates)
{
    const auto head_predicates = compute_predicate_head_occurrences(axioms);

    auto R = std::unordered_map<Predicate<DerivedTag>, std::unordered_map<Predicate<DerivedTag>, StratumStatus>> {};

    // lines 2-4
    for (const auto predicate_1 : derived_predicates)
    {
        for (const auto predicate_2 : derived_predicates)
        {
            R[predicate_1][predicate_2] = StratumStatus::UNCONSTRAINED;
        }
    }

    // lines 5-10
    for (const auto& axiom : axioms)
    {
        const auto head_predicate = axiom->get_literal()->get_atom()->get_predicate();

        for (const auto& condition : axiom->get_conjunctive_condition()->get_literals<DerivedTag>())
        {
            const auto condition_predicate = condition->get_atom()->get_predicate();
            if (!condition->get_polarity())
            {
                R[condition_predicate][head_predicate] = StratumStatus::STRICTLY_LOWER;
            }
            else
            {
                R[condition_predicate][head_predicate] = StratumStatus::LOWER;
            }
        }
    }

    // lines 11-15
    for (const auto& predicate_1 : derived_predicates)
    {
        for (const auto& predicate_2 : derived_predicates)
        {
            for (const auto& predicate_3 : derived_predicates)
            {
                if (std::min(static_cast<int>(R.at(predicate_2).at(predicate_1)), static_cast<int>(R.at(predicate_1).at(predicate_3))) > 0)
                {
                    R.at(predicate_2).at(predicate_3) = static_cast<StratumStatus>(std::max({ static_cast<int>(R.at(predicate_2).at(predicate_1)),
                                                                                              static_cast<int>(R.at(predicate_1).at(predicate_3)),
                                                                                              static_cast<int>(R.at(predicate_2).at(predicate_3)) }));
                }
            }
        }
    }

    // lines 16-27
    if (std::any_of(derived_predicates.begin(),
                    derived_predicates.end(),
                    [&R](const auto& predicate) { return R.at(predicate).at(predicate) == StratumStatus::STRICTLY_LOWER; }))
    {
        throw std::runtime_error("Set of axioms is not stratifiable.");
    }

    auto stratification = std::vector<PredicateSet<DerivedTag>> {};
    auto remaining = PredicateSet<DerivedTag>(derived_predicates.begin(), derived_predicates.end());
    while (!remaining.empty())
    {
        auto stratum = PredicateSet<DerivedTag> {};
        for (const auto& predicate_1 : remaining)
        {
            if (std::all_of(remaining.begin(),
                            remaining.end(),
                            [&R, &predicate_1](const auto& predicate_2) { return R.at(predicate_2).at(predicate_1) != StratumStatus::STRICTLY_LOWER; }))
            {
                stratum.insert(predicate_1);
            }
        }

        for (const auto& predicate : stratum)
        {
            remaining.erase(predicate);
        }

        stratification.push_back(std::move(stratum));
    }

    return stratification;
}

std::vector<AxiomPartition> compute_axiom_partitioning(const AxiomList& axioms, const PredicateList<DerivedTag>& derived_predicates)
{
    const auto stratification = compute_stratification(axioms, derived_predicates);

    const auto derived_predicate_set = PredicateSet<DerivedTag>(derived_predicates.begin(), derived_predicates.end());

    auto axiom_partitioning = std::vector<AxiomPartition> {};

    auto remaining_axioms = AxiomSet(axioms.begin(), axioms.end());
    auto affected_derived_predicates_in_earlier_partition = PredicateSet<DerivedTag> {};
    for (const auto& stratum : stratification)
    {
        auto partition = AxiomSet {};
        for (const auto& axiom : remaining_axioms)
        {
            if (stratum.count(axiom->get_literal()->get_atom()->get_predicate()))
            {
                partition.insert(axiom);
            }
        }
        for (const auto& axiom : partition)
        {
            remaining_axioms.erase(axiom);
        }

        axiom_partitioning.push_back(AxiomPartition(partition, derived_predicate_set, affected_derived_predicates_in_earlier_partition));

        for (const auto& axiom : partition)
        {
            affected_derived_predicates_in_earlier_partition.insert(axiom->get_literal()->get_atom()->get_predicate());
        }
    }

    return axiom_partitioning;
}

}