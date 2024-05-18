#include "mimir/search/axiom_evaluators/axiom_stratification.hpp"

#include "mimir/common/printers.hpp"
#include "mimir/formalism/atom.hpp"
#include "mimir/formalism/axiom.hpp"
#include "mimir/formalism/ground_atom.hpp"
#include "mimir/formalism/literal.hpp"
#include "mimir/formalism/predicate.hpp"

#include <algorithm>

namespace mimir
{

AxiomPartition::AxiomPartition(AxiomSet axioms) : m_axioms(std::move(axioms)), m_axioms_by_body_predicates()
{
    for (const auto& axiom : m_axioms)
    {
        for (const auto& literal : axiom->get_fluent_conditions())
        {
            m_axioms_by_body_predicates[literal->get_atom()->get_predicate()].insert(axiom);
        }
    }
}

void AxiomPartition::retrieve_axioms_with_same_body_predicate(GroundAtom derived_atom, AxiomSet& ref_axioms) const
{
    auto it = m_axioms_by_body_predicates.find(derived_atom->get_predicate());
    if (it != m_axioms_by_body_predicates.end())
    {
        const auto& relevant_axioms = it->second;

        ref_axioms.insert(relevant_axioms.begin(), relevant_axioms.end());
    }
}

const AxiomSet& AxiomPartition::get_axioms() const { return m_axioms; }

/// @brief Compute occurrences of predicates in axiom heads
/// @param axioms a set of axioms
/// @param predicates the set of all predicates occuring in axioms
static std::unordered_map<Predicate, AxiomList> compute_predicate_head_occurrences(const AxiomList& axioms)
{
    auto predicate_occurrences = std::unordered_map<Predicate, AxiomList> {};
    for (const auto& axiom : axioms)
    {
        predicate_occurrences[axiom->get_literal()->get_atom()->get_predicate()].push_back(axiom);
    }
    return predicate_occurrences;
}

/// @brief Compute positive occurrences of predicates in axiom bodies
/// @param axioms a set of axioms
/// @param predicates the set of all predicates occuring in axioms
static std::unordered_map<Predicate, AxiomList> compute_predicate_body_positive_occurrences(const AxiomList& axioms)
{
    auto predicate_occurrences = std::unordered_map<Predicate, AxiomList> {};
    for (const auto& axiom : axioms)
    {
        for (const auto& condition : axiom->get_conditions())
        {
            if (!condition->is_negated())
            {
                predicate_occurrences[condition->get_atom()->get_predicate()].push_back(axiom);
            }
        }
    }
    return predicate_occurrences;
}

/// @brief Compute negative occurrences of predicates in axiom bodies
/// @param axioms a set of axioms
/// @param predicates the set of all predicates occuring in axioms
static std::unordered_map<Predicate, AxiomList> compute_predicate_body_negative_occurrences(const AxiomList& axioms)
{
    auto predicate_occurrences = std::unordered_map<Predicate, AxiomList> {};
    for (const auto& axiom : axioms)
    {
        for (const auto& condition : axiom->get_conditions())
        {
            if (condition->is_negated())
            {
                predicate_occurrences[condition->get_atom()->get_predicate()].push_back(axiom);
            }
        }
    }
    return predicate_occurrences;
}

enum class StratumStatus
{
    UNCONSTRAINED = 0,
    LOWER = 1,
    STRICTLY_LOWER = 2,
};

static std::vector<PredicateSet> compute_stratification(const AxiomList& axioms, const PredicateList& derived_predicates)
{
    const auto head_predicates = compute_predicate_head_occurrences(axioms);
    const auto body_positive_predicates = compute_predicate_body_positive_occurrences(axioms);
    const auto body_negative_predicates = compute_predicate_body_negative_occurrences(axioms);

    auto R = std::unordered_map<Predicate, std::unordered_map<Predicate, StratumStatus>> {};

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

        for (const auto& condition : axiom->get_conditions())
        {
            const auto condition_predicate = condition->get_atom()->get_predicate();
            if (condition->is_negated())
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

    auto stratification = std::vector<PredicateSet> {};
    auto remaining = PredicateSet(derived_predicates.begin(), derived_predicates.end());
    while (!remaining.empty())
    {
        auto stratum = PredicateSet {};
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

        stratification.push_back(move(stratum));
    }

    return stratification;
}

std::vector<AxiomPartition> compute_axiom_partitioning(const AxiomList& axioms, const PredicateList& derived_predicates)
{
    const auto stratification = compute_stratification(axioms, derived_predicates);

    const auto derived_predicate_set = PredicateSet(derived_predicates.begin(), derived_predicates.end());

    auto axiom_partitioning = std::vector<AxiomPartition> {};

    auto remaining_axioms = AxiomSet(axioms.begin(), axioms.end());
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
        axiom_partitioning.push_back(AxiomPartition(std::move(partition)));
    }

    return axiom_partitioning;
}

}