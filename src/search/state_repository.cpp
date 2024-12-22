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
#include "mimir/search/workspaces/axiom_evaluator.hpp"
#include "mimir/search/workspaces/state_repository.hpp"

namespace mimir
{

StateRepository::StateRepository(std::shared_ptr<IAxiomEvaluator> axiom_evaluator) :
    m_axiom_evaluator(std::move(axiom_evaluator)),
    m_problem_or_domain_has_axioms(!m_axiom_evaluator->get_axiom_grounder()->get_problem()->get_axioms().empty()
                                   || !m_axiom_evaluator->get_axiom_grounder()->get_problem()->get_domain()->get_axioms().empty()),
    m_states(),
    m_axiom_evaluations(),
    m_reached_fluent_atoms(),
    m_reached_derived_atoms()
{
}

State StateRepository::get_or_create_initial_state(StateRepositoryWorkspace& workspace)
{
    auto ground_atoms = GroundAtomList<Fluent> {};

    for (const auto& literal : m_axiom_evaluator->get_axiom_grounder()->get_problem()->get_fluent_initial_literals())
    {
        ground_atoms.push_back(literal->get_atom());
    }

    return get_or_create_state(ground_atoms, workspace);
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

static void update_derived_atoms_ptr(const FlatIndexList& state_derived_atoms, uintptr_t& state_derived_atoms_ptr)
{
    state_derived_atoms_ptr = reinterpret_cast<uintptr_t>(&state_derived_atoms);
}

State StateRepository::get_or_create_state(const GroundAtomList<Fluent>& atoms, StateRepositoryWorkspace& workspace)
{
    auto& state_builder = workspace.get_or_create_state_builder();
    auto& state_fluent_atoms = state_builder.get_fluent_atoms();
    auto& state_derived_atoms = workspace.get_or_create_new_derived_atoms_list();
    auto& state_derived_atoms_ptr = state_builder.get_derived_atoms();
    state_fluent_atoms.clear();
    state_derived_atoms.clear();
    state_derived_atoms_ptr = uintptr_t(0);

    auto& dense_state = workspace.get_or_create_dense_state();
    auto& dense_fluent_atoms = dense_state.get_atoms<Fluent>();
    auto& dense_derived_atoms = dense_state.get_atoms<Derived>();
    dense_fluent_atoms.unset_all();
    dense_derived_atoms.unset_all();

    /* 1. Set state index */

    {
        auto& state_index = state_builder.get_index();
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
        auto iter = m_states.find(state_builder);
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
            m_axiom_evaluator->generate_and_apply_axioms(dense_state, workspace.get_or_create_axiom_evaluator_workspace());

            update_reached_derived_atoms(dense_derived_atoms, m_reached_derived_atoms);

            translate_dense_into_sorted_compressed_sparse(dense_derived_atoms, state_derived_atoms);

            const auto [iter, inserted] = m_axiom_evaluations.insert(state_derived_atoms);

            update_derived_atoms_ptr(**iter, state_derived_atoms_ptr);
        }
    }

    // Cache and return the extended state.
    return m_states.insert(state_builder).first->get();
}

static void apply_action_effects(const FlatBitset& negative_applied_effects, const FlatBitset& positive_applied_effects, FlatBitset& ref_state_fluent_atoms)
{
    ref_state_fluent_atoms -= negative_applied_effects;
    ref_state_fluent_atoms |= positive_applied_effects;
}

std::pair<State, ContinuousCost> StateRepository::get_or_create_successor_state(State state, GroundAction action, StateRepositoryWorkspace& workspace)
{
    auto& dense_state = workspace.get_or_create_dense_state();
    auto& dense_fluent_atoms = dense_state.get_atoms<Fluent>();
    auto& dense_derived_atoms = dense_state.get_atoms<Derived>();
    dense_fluent_atoms.unset_all();
    dense_derived_atoms.unset_all();

    insert_into_bitset(state->get_atoms<Fluent>(), dense_fluent_atoms);
    insert_into_bitset(state->get_atoms<Derived>(), dense_derived_atoms);

    return get_or_create_successor_state(dense_state, action, workspace);
}

static void collect_applied_strips_effects(GroundAction action,
                                           FlatBitset& ref_negative_applied_effects,
                                           FlatBitset& ref_positive_applied_effects,
                                           ContinuousCost& ref_action_cost)
{
    const auto& strips_action_effect = action->get_strips_effect();
    insert_into_bitset(strips_action_effect.get_negative_effects(), ref_negative_applied_effects);
    insert_into_bitset(strips_action_effect.get_positive_effects(), ref_positive_applied_effects);
    ref_action_cost += strips_action_effect.get_cost();
}

static void collect_applied_conditional_effects(GroundAction action,
                                                Problem problem,
                                                const DenseState& dense_state,
                                                FlatBitset& ref_negative_applied_effects,
                                                FlatBitset& ref_positive_applied_effects,
                                                ContinuousCost& ref_action_cost)
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

std::pair<State, ContinuousCost>
StateRepository::get_or_create_successor_state(DenseState& dense_state, GroundAction action, StateRepositoryWorkspace& workspace)
{
    auto& dense_fluent_atoms = dense_state.get_atoms<Fluent>();
    auto& dense_derived_atoms = dense_state.get_atoms<Derived>();

    auto& state_builder = workspace.get_or_create_state_builder();
    auto& state_fluent_atoms = state_builder.get_fluent_atoms();
    auto& state_derived_atoms = workspace.get_or_create_new_derived_atoms_list();
    auto& state_derived_atoms_ptr = state_builder.get_derived_atoms();
    state_fluent_atoms.clear();
    state_derived_atoms.clear();
    state_derived_atoms_ptr = uintptr_t(0);

    /* Accumulate state-dependent action cost. */
    auto action_cost = ContinuousCost(0);

    /* 1. Set the state index. */
    {
        auto& state_index = state_builder.get_index();
        const auto next_state_index = Index(m_states.size());
        state_index = next_state_index;
    }

    /* 2. Apply action effects to construct non-extended state. */
    {
        auto& negative_applied_effects = workspace.get_or_create_applied_negative_effect_atoms();
        auto& positive_applied_effects = workspace.get_or_create_applied_positive_effect_atoms();
        negative_applied_effects.unset_all();
        positive_applied_effects.unset_all();

        collect_applied_strips_effects(action, negative_applied_effects, positive_applied_effects, action_cost);

        collect_applied_conditional_effects(action,
                                            m_axiom_evaluator->get_axiom_grounder()->get_problem(),
                                            dense_state,
                                            negative_applied_effects,
                                            positive_applied_effects,
                                            action_cost);

        apply_action_effects(negative_applied_effects, positive_applied_effects, dense_fluent_atoms);

        update_reached_fluent_atoms(dense_fluent_atoms, m_reached_fluent_atoms);

        translate_dense_into_sorted_compressed_sparse(dense_fluent_atoms, state_fluent_atoms);

        // Check if non-extended state exists in cache
        const auto iter = m_states.find(state_builder);
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
            m_axiom_evaluator->generate_and_apply_axioms(dense_state, workspace.get_or_create_axiom_evaluator_workspace());

            update_reached_fluent_atoms(dense_derived_atoms, m_reached_derived_atoms);

            translate_dense_into_sorted_compressed_sparse(dense_derived_atoms, state_derived_atoms);

            const auto [iter, inserted] = m_axiom_evaluations.insert(state_derived_atoms);

            update_derived_atoms_ptr(**iter, state_derived_atoms_ptr);
        }
    }

    // Cache and return the extended state.
    return std::make_pair(m_states.insert(state_builder).first->get(), action_cost);
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
