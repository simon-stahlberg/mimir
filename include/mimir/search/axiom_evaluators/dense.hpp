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

#ifndef MIMIR_SEARCH_AXIOM_EVALUATORS_DENSE_LIFTED_HPP_
#define MIMIR_SEARCH_AXIOM_EVALUATORS_DENSE_LIFTED_HPP_

#include "mimir/common/printers.hpp"
#include "mimir/formalism/formalism.hpp"
#include "mimir/formalism/predicate_category.hpp"
#include "mimir/formalism/problem.hpp"
#include "mimir/search/applicable_action_generators/dense_lifted/assignment_set.hpp"
#include "mimir/search/applicable_action_generators/dense_lifted/consistency_graph.hpp"
#include "mimir/search/applicable_action_generators/dense_lifted/event_handlers.hpp"
#include "mimir/search/axiom_evaluators/axiom_stratification.hpp"
#include "mimir/search/axiom_evaluators/interface.hpp"
#include "mimir/search/axioms.hpp"
#include "mimir/search/condition_grounders.hpp"
#include "mimir/search/states.hpp"

#include <flatmemory/details/view_const.hpp>
#include <stdexcept>
#include <unordered_map>
#include <vector>

namespace mimir
{

class PartiallyExtendedState
{
private:
    const FlatBitsetBuilder<Fluent>& m_fluent_state_atoms;
    FlatBitsetBuilder<Derived>& m_ref_derived_state_atoms;

public:
    PartiallyExtendedState(const FlatBitsetBuilder<Fluent>& fluent_state_atoms, FlatBitsetBuilder<Derived>& ref_derived_state_atoms) :
        m_fluent_state_atoms(fluent_state_atoms),
        m_ref_derived_state_atoms(ref_derived_state_atoms)
    {
    }

    bool literal_holds(GroundLiteral<Fluent> fluent_literal) const
    {
        return m_fluent_state_atoms.get(fluent_literal->get_atom()->get_identifier()) != fluent_literal->is_negated();
    }

    bool literal_holds(GroundLiteral<Derived> derived_literal) const
    {
        return m_ref_derived_state_atoms.get(derived_literal->get_atom()->get_identifier()) != derived_literal->is_negated();
    }
};

/**
 * Fully specialized implementation class.
 */
template<>
class AE<DenseStateTag> : public IStaticAE<AE<DenseStateTag>>
{
private:
    Problem m_problem;
    PDDLFactories& m_ref_pddl_factories;
    std::shared_ptr<ILiftedAAGEventHandler> m_event_handler;

    std::vector<AxiomPartition> m_partitioning;

    FlatDenseAxiomSet m_flat_dense_axioms;
    DenseGroundAxiomList m_axioms_by_index;
    DenseGroundAxiomBuilder m_axiom_builder;
    std::unordered_map<Axiom, GroundingTable<DenseGroundAxiom>> m_axiom_groundings;

    // std::unordered_map<Axiom, consistency_graph::StaticConsistencyGraph> m_static_consistency_graphs;

    std::unordered_map<Axiom, ConditionGrounder<PartiallyExtendedState>> m_condition_grounders;

    /* Implement IStaticAE interface */
    friend class IStaticAE<AE<DenseStateTag>>;

    void generate_and_apply_axioms_impl(const FlatBitsetBuilder<Fluent>& fluent_state_atoms, FlatBitsetBuilder<Derived>& ref_derived_state_atoms);

public:
    /// @brief Simplest construction, expects the event handler from the lifted aag.
    AE(Problem problem, PDDLFactories& pddl_factories, std::shared_ptr<ILiftedAAGEventHandler> event_handler);

    /// @brief Return the axiom partitioning.
    [[nodiscard]] const std::vector<AxiomPartition>& get_axiom_partitioning() const;

    /// @brief Ground an axiom and return a view onto it.
    [[nodiscard]] DenseGroundAxiom ground_axiom(const Axiom& axiom, ObjectList&& binding);

    /// @brief Return all axioms.
    [[nodiscard]] const DenseGroundAxiomList& get_dense_ground_axioms() const;
};

/**
 * Types
 */

using DenseAE = AE<DenseStateTag>;

}  // namespace mimir

#endif
