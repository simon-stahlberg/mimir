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

#include "mimir/search/state_repository.hpp"

#include "mimir/common/types_cista.hpp"
#include "mimir/formalism/domain.hpp"
#include "mimir/formalism/ground_atom.hpp"
#include "mimir/formalism/ground_literal.hpp"
#include "mimir/formalism/problem.hpp"
#include "mimir/search/action.hpp"
#include "mimir/search/axiom_evaluators/interface.hpp"
#include "mimir/search/grounders/axiom_grounder.hpp"

namespace mimir
{

StateRepository::StateRepository(std::shared_ptr<IAxiomEvaluator> axiom_evaluator) :
    m_axiom_evaluator(std::move(axiom_evaluator)),
    m_problem_or_domain_has_axioms(!m_axiom_evaluator->get_axiom_grounder()->get_problem()->get_axioms().empty()
                                   || !m_axiom_evaluator->get_axiom_grounder()->get_problem()->get_domain()->get_axioms().empty()),
    m_states(),
    m_axiom_evaluations(),
    m_reached_fluent_atoms(),
    m_reached_derived_atoms(),
    m_state_builder(),
    m_dense_state_builder(),
    m_applied_positive_effect_atoms(),
    m_applied_negative_effect_atoms(),
    m_state_derived_atoms()
{
}

State StateRepository::get_or_create_initial_state()
{
    return get_or_create_state(m_axiom_evaluator->get_axiom_grounder()->get_problem()->get_fluent_initial_atoms());
}

static void translate_dense_into_sorted_compressed_sparse(const FlatBitset& dense, FlatIndexList& ref_sparse)
{
    insert_into_vector(dense, ref_sparse);
    ref_sparse.compress();
}

static void update_reached_fluent_atoms(const FlatBitset& state_fluent_atoms, FlatBitset& ref_reached_fluent_atoms)
{
    ref_reached_fluent_atoms |= state_fluent_atoms;
}

static void update_reached_derived_atoms(const FlatBitset& state_derived_atoms, FlatBitset& ref_reached_derived_atoms)
{
    ref_reached_derived_atoms |= state_derived_atoms;
}

static void update_derived_atoms_ptr(const FlatIndexList& state_derived_atoms, FlatExternalPtr<const FlatIndexList>& state_derived_atoms_ptr)
{
    state_derived_atoms_ptr = &state_derived_atoms;
}

State StateRepository::get_or_create_state(const GroundAtomList<Fluent>& atoms)
{
    auto& state_fluent_atoms = m_state_builder.get_fluent_atoms();
    auto& state_derived_atoms_ptr = m_state_builder.get_derived_atoms();
    state_fluent_atoms.clear();
    m_state_derived_atoms.clear();
    state_derived_atoms_ptr = nullptr;

    auto& dense_fluent_atoms = m_dense_state_builder.get_atoms<Fluent>();
    auto& dense_derived_atoms = m_dense_state_builder.get_atoms<Derived>();
    dense_fluent_atoms.unset_all();
    dense_derived_atoms.unset_all();

    /* 1. Set state index */

    {
        auto& state_index = m_state_builder.get_index();
        const auto next_state_index = Index(m_states.size());
        state_index = next_state_index;
    }

    /* 2. Construct non-extended state */

    {
        for (const auto& atom : atoms)
        {
            dense_fluent_atoms.set(atom->get_index());
        }

        update_reached_fluent_atoms(dense_fluent_atoms, m_reached_fluent_atoms);

        translate_dense_into_sorted_compressed_sparse(dense_fluent_atoms, state_fluent_atoms);

        // Test whether there exists an extended state for the given non extended state
        auto iter = m_states.find(m_state_builder);
        if (iter != m_states.end())
        {
            return iter->get();
        }
    }

    /* 3. Apply axioms to construct extended state. */
    {
        if (m_problem_or_domain_has_axioms)
        {
            // Evaluate axioms
            assert(dense_derived_atoms.count() == 0);
            m_axiom_evaluator->generate_and_apply_axioms(m_dense_state_builder);

            update_reached_derived_atoms(dense_derived_atoms, m_reached_derived_atoms);

            translate_dense_into_sorted_compressed_sparse(dense_derived_atoms, m_state_derived_atoms);

            const auto [iter, inserted] = m_axiom_evaluations.insert(m_state_derived_atoms);

            update_derived_atoms_ptr(**iter, state_derived_atoms_ptr);
        }
    }

    // Cache and return the extended state.
    return m_states.insert(m_state_builder).first->get();
}

static void apply_action_effects(const FlatBitset& negative_applied_effects, const FlatBitset& positive_applied_effects, FlatBitset& ref_state_fluent_atoms)
{
    ref_state_fluent_atoms -= negative_applied_effects;
    ref_state_fluent_atoms |= positive_applied_effects;
}

std::pair<State, ContinuousCost> StateRepository::get_or_create_successor_state(State state, GroundAction action)
{
    DenseState::translate(state, m_dense_state_builder);

    return get_or_create_successor_state(m_dense_state_builder, action);
}

static void collect_applied_strips_effects(GroundAction action,
                                           FlatBitset& ref_negative_applied_effects,
                                           FlatBitset& ref_positive_applied_effects,
                                           absl::flat_hash_map<Index, ContinuousCost>& auxilary_function_to_cost)
{
    const auto& strips_action_effect = action->get_strips_effect();
    insert_into_bitset(strips_action_effect.get_negative_effects(), ref_negative_applied_effects);
    insert_into_bitset(strips_action_effect.get_positive_effects(), ref_positive_applied_effects);
    for (const auto& ground_numeric_effect : strips_action_effect.get_numeric_effects<Auxiliary>()) {}
}

static void collect_applied_conditional_effects(GroundAction action,
                                                Problem problem,
                                                const DenseState& dense_state,
                                                FlatBitset& ref_negative_applied_effects,
                                                FlatBitset& ref_positive_applied_effects,
                                                absl::flat_hash_map<Index, ContinuousCost>& auxilary_function_to_cost)
{
    for (const auto& conditional_effect : action->get_conditional_effects())
    {
        if (conditional_effect.is_applicable(problem, dense_state))
        {
            for (const auto& effect_literal : conditional_effect.get_fluent_effect_literals())
            {
                if (effect_literal.is_negated)
                {
                    ref_negative_applied_effects.set(effect_literal.atom_index);
                }
                else
                {
                    ref_positive_applied_effects.set(effect_literal.atom_index);
                }
            }
            ref_action_cost += conditional_effect.get_cost();
        }
    }
}

std::pair<State, ContinuousCost> StateRepository::get_or_create_successor_state(DenseState& dense_state, GroundAction action)
{
    const auto& metric = m_axiom_evaluator->get_problem()->get_optimization_metric();

    m_auxilary_function_to_cost.clear();

    auto& dense_fluent_atoms = dense_state.get_atoms<Fluent>();
    auto& dense_derived_atoms = dense_state.get_atoms<Derived>();

    auto& state_fluent_atoms = m_state_builder.get_fluent_atoms();
    auto& state_derived_atoms_ptr = m_state_builder.get_derived_atoms();
    state_fluent_atoms.clear();
    m_state_derived_atoms.clear();
    state_derived_atoms_ptr = nullptr;

    /* 1. Set the state index. */
    {
        auto& state_index = m_state_builder.get_index();
        const auto next_state_index = Index(m_states.size());
        state_index = next_state_index;
    }

    /* 2. Apply action effects to construct non-extended state. */
    {
        m_applied_negative_effect_atoms.unset_all();
        m_applied_positive_effect_atoms.unset_all();

        collect_applied_strips_effects(action, m_applied_negative_effect_atoms, m_applied_positive_effect_atoms, m_auxilary_function_to_cost);

        collect_applied_conditional_effects(action,
                                            m_axiom_evaluator->get_axiom_grounder()->get_problem(),
                                            dense_state,
                                            m_applied_negative_effect_atoms,
                                            m_applied_positive_effect_atoms,
                                            m_auxilary_function_to_cost);

        apply_action_effects(m_applied_negative_effect_atoms, m_applied_positive_effect_atoms, dense_fluent_atoms);

        update_reached_fluent_atoms(dense_fluent_atoms, m_reached_fluent_atoms);

        translate_dense_into_sorted_compressed_sparse(dense_fluent_atoms, state_fluent_atoms);

        // Check if non-extended state exists in cache
        const auto iter = m_states.find(m_state_builder);
        if (iter != m_states.end())
        {
            return std::make_pair(iter->get(), action_cost);
        }
    }

    /* 3. If necessary, apply axioms to construct extended state. */
    {
        if (m_problem_or_domain_has_axioms)
        {
            // Evaluate axioms
            dense_derived_atoms.unset_all();  ///< Important: now we must clear the buffer before evaluating for the updated fluent atoms.
            m_axiom_evaluator->generate_and_apply_axioms(dense_state);

            update_reached_fluent_atoms(dense_derived_atoms, m_reached_derived_atoms);

            translate_dense_into_sorted_compressed_sparse(dense_derived_atoms, m_state_derived_atoms);

            const auto [iter, inserted] = m_axiom_evaluations.insert(m_state_derived_atoms);

            update_derived_atoms_ptr(**iter, state_derived_atoms_ptr);
        }
    }

    // Cache and return the extended state.
    return std::make_pair(m_states.insert(m_state_builder).first->get(), action_cost);
}

Problem StateRepository::get_problem() const { return m_axiom_evaluator->get_problem(); }

const std::shared_ptr<PDDLRepositories>& StateRepository::get_pddl_repositories() const { return m_axiom_evaluator->get_pddl_repositories(); }

size_t StateRepository::get_state_count() const { return m_states.size(); }

const FlatBitset& StateRepository::get_reached_fluent_ground_atoms_bitset() const { return m_reached_fluent_atoms; }

const FlatBitset& StateRepository::get_reached_derived_ground_atoms_bitset() const { return m_reached_derived_atoms; }

const std::shared_ptr<IAxiomEvaluator>& StateRepository::get_axiom_evaluator() const { return m_axiom_evaluator; }

size_t StateRepository::get_estimated_memory_usage_in_bytes_for_unextended_state_portion() const { return m_states.get_estimated_memory_usage_in_bytes(); }

size_t StateRepository::get_estimated_memory_usage_in_bytes_for_extended_state_portion() const
{
    return m_axiom_evaluations.get_estimated_memory_usage_in_bytes();
}
}
