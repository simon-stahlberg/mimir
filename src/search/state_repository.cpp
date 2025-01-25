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
#include "mimir/formalism/ground_action.hpp"
#include "mimir/formalism/ground_atom.hpp"
#include "mimir/formalism/ground_literal.hpp"
#include "mimir/formalism/grounders/axiom_grounder.hpp"
#include "mimir/formalism/problem.hpp"
#include "mimir/formalism/repositories.hpp"
#include "mimir/search/applicability.hpp"
#include "mimir/search/axiom_evaluators/interface.hpp"

namespace mimir
{

StateRepository::StateRepository(std::shared_ptr<IAxiomEvaluator> axiom_evaluator) :
    m_axiom_evaluator(std::move(axiom_evaluator)),
    m_problem_or_domain_has_axioms(!m_axiom_evaluator->get_axiom_grounder()->get_problem()->get_axioms().empty()
                                   || !m_axiom_evaluator->get_axiom_grounder()->get_problem()->get_domain()->get_axioms().empty()),
    m_states(),
    m_derived_atoms_set(),
    m_reached_fluent_atoms(),
    m_reached_derived_atoms(),
    m_state_builder(),
    m_dense_state_builder(),
    m_applied_positive_effect_atoms(),
    m_applied_negative_effect_atoms(),
    m_state_fluent_atoms(),
    m_state_derived_atoms(),
    m_state_auxiliary_numeric_variables()
{
}

std::pair<State, const FlatDoubleList*> StateRepository::get_or_create_initial_state()
{
    const auto problem = m_axiom_evaluator->get_axiom_grounder()->get_problem();
    return get_or_create_state(problem->get_fluent_initial_atoms(), problem->get_function_to_value<Fluent>());
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

static void update_state_atoms_ptr(const FlatIndexList& state_atoms, FlatExternalPtr<const FlatIndexList>& state_atoms_ptr) { state_atoms_ptr = &state_atoms; }

static void update_state_numeric_variables_ptr(const FlatDoubleList& state_numeric_variables,
                                               FlatExternalPtr<const FlatDoubleList>& state_numeric_variables_ptr)
{
    state_numeric_variables_ptr = &state_numeric_variables;
}

std::pair<State, const FlatDoubleList*> StateRepository::get_or_create_state(const GroundAtomList<Fluent>& atoms,
                                                                             const FlatDoubleList& fluent_numeric_variables)
{
    // Fetch data and initialize it
    auto& state_fluent_atoms_ptr = m_state_builder.get_fluent_atoms();
    auto& state_derived_atoms_ptr = m_state_builder.get_derived_atoms();
    auto& state_fluent_numeric_variables_ptr = m_state_builder.get_numeric_variables();
    m_state_fluent_atoms.clear();
    m_state_derived_atoms.clear();
    state_fluent_atoms_ptr = nullptr;
    state_derived_atoms_ptr = nullptr;
    state_fluent_numeric_variables_ptr = nullptr;

    auto& dense_fluent_atoms = m_dense_state_builder.get_atoms<Fluent>();
    auto& dense_derived_atoms = m_dense_state_builder.get_atoms<Derived>();
    auto& dense_numeric_variables = m_dense_state_builder.get_numeric_variables();
    dense_fluent_atoms.unset_all();
    dense_derived_atoms.unset_all();

    const auto problem = m_axiom_evaluator->get_problem();
    m_state_auxiliary_numeric_variables.clear();

    /* 1. Set state index */

    {
        auto& state_index = m_state_builder.get_index();
        const auto next_state_index = Index(m_states.size());
        state_index = next_state_index;
    }

    /* 2. Construct non-extended state */

    /* 2.1 Numeric state variables */
    dense_numeric_variables = fluent_numeric_variables;
    m_state_auxiliary_numeric_variables = problem->get_function_to_value<Auxiliary>();
    const auto [fluent_numeric_iter, fluent_numeric_inserted] = m_fluent_numeric_variables_set.insert(dense_numeric_variables);
    const auto [auxiliary_numeric_iter, auxiliary_numeric_inserted] = m_auxiliary_numeric_variables_set.insert(m_state_auxiliary_numeric_variables);

    update_state_numeric_variables_ptr(**fluent_numeric_iter, state_fluent_numeric_variables_ptr);

    /* 2.2. Propositional state */
    {
        for (const auto& atom : atoms)
        {
            dense_fluent_atoms.set(atom->get_index());
        }

        update_reached_fluent_atoms(dense_fluent_atoms, m_reached_fluent_atoms);

        translate_dense_into_sorted_compressed_sparse(dense_fluent_atoms, m_state_fluent_atoms);

        const auto [fluent_iter, fluent_inserted] = m_fluent_atoms_set.insert(m_state_fluent_atoms);
        update_state_atoms_ptr(**fluent_iter, state_fluent_atoms_ptr);

        // Test whether there exists an extended state for the given non extended state
        auto state_iter = m_states.find(m_state_builder);
        if (state_iter != m_states.end())
        {
            return { state_iter->get(), auxiliary_numeric_iter->get() };
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

            const auto [derived_iter, derived_inserted] = m_derived_atoms_set.insert(m_state_derived_atoms);
            update_state_atoms_ptr(**derived_iter, state_derived_atoms_ptr);
        }
    }

    // Cache and return the extended state.
    return { m_states.insert(m_state_builder).first->get(), auxiliary_numeric_iter->get() };
}

static void apply_action_effects(const FlatBitset& negative_applied_effects, const FlatBitset& positive_applied_effects, FlatBitset& ref_state_fluent_atoms)
{
    ref_state_fluent_atoms -= negative_applied_effects;
    ref_state_fluent_atoms |= positive_applied_effects;
}

std::pair<State, const FlatDoubleList*>
StateRepository::get_or_create_successor_state(State state, GroundAction action, const FlatDoubleList& auxiliary_numeric_state_variables)
{
    DenseState::translate(state, m_dense_state_builder);

    return get_or_create_successor_state(m_dense_state_builder, action, auxiliary_numeric_state_variables);
}

static void collect_applied_conjunctive_effects(GroundAction action,
                                                const FlatDoubleList& state_fluent_numeric_variables,
                                                const FlatDoubleList& state_auxiliary_numeric_variables,
                                                FlatBitset& ref_negative_applied_effects,
                                                FlatBitset& ref_positive_applied_effects,
                                                FlatDoubleList& ref_fluent_numeric_variables,
                                                FlatDoubleList& ref_auxiliary_numeric_variables)
{
    const auto& conjunctive_effect = action->get_conjunctive_effect();
    insert_into_bitset(conjunctive_effect.get_negative_effects(), ref_negative_applied_effects);
    insert_into_bitset(conjunctive_effect.get_positive_effects(), ref_positive_applied_effects);
    for (const auto& numeric_effect : conjunctive_effect.get_numeric_effects<Fluent>())
    {
        const auto index = numeric_effect.get_function()->get_index();
        if (index >= ref_fluent_numeric_variables.size())
        {
            ref_fluent_numeric_variables.resize(index + 1, UNDEFINED_CONTINUOUS_COST);
        }

        ref_fluent_numeric_variables[index] = evaluate(numeric_effect, state_fluent_numeric_variables, state_auxiliary_numeric_variables);
    }
    for (const auto& numeric_effect : conjunctive_effect.get_numeric_effects<Auxiliary>())
    {
        const auto index = numeric_effect.get_function()->get_index();
        if (index >= ref_auxiliary_numeric_variables.size())
        {
            ref_auxiliary_numeric_variables.resize(index + 1, UNDEFINED_CONTINUOUS_COST);
        }

        ref_auxiliary_numeric_variables[index] = evaluate(numeric_effect, state_fluent_numeric_variables, state_auxiliary_numeric_variables);
    }
}

static void collect_applied_conditional_effects(GroundAction action,
                                                const FlatDoubleList& state_fluent_numeric_variables,
                                                const FlatDoubleList& state_auxiliary_numeric_variables,
                                                Problem problem,
                                                const DenseState& dense_state,
                                                FlatBitset& ref_negative_applied_effects,
                                                FlatBitset& ref_positive_applied_effects,
                                                FlatDoubleList& ref_fluent_numeric_variables,
                                                FlatDoubleList& ref_auxiliary_numeric_variables)
{
    for (const auto& conditional_effect : action->get_conditional_effects())
    {
        if (is_applicable(conditional_effect, problem, dense_state))
        {
            insert_into_bitset(conditional_effect.get_conjunctive_effect().get_positive_effects(), ref_positive_applied_effects);
            insert_into_bitset(conditional_effect.get_conjunctive_effect().get_negative_effects(), ref_negative_applied_effects);
            for (const auto& numeric_effect : conditional_effect.get_conjunctive_effect().get_numeric_effects<Fluent>())
            {
                const auto index = numeric_effect.get_function()->get_index();
                if (index >= ref_fluent_numeric_variables.size())
                {
                    ref_fluent_numeric_variables.resize(index + 1, UNDEFINED_CONTINUOUS_COST);
                }

                ref_fluent_numeric_variables[index] = evaluate(numeric_effect, state_fluent_numeric_variables, state_auxiliary_numeric_variables);
            }
            for (const auto& numeric_effect : conditional_effect.get_conjunctive_effect().get_numeric_effects<Auxiliary>())
            {
                const auto index = numeric_effect.get_function()->get_index();
                if (index >= ref_auxiliary_numeric_variables.size())
                {
                    ref_auxiliary_numeric_variables.resize(index + 1, UNDEFINED_CONTINUOUS_COST);
                }

                ref_auxiliary_numeric_variables[index] = evaluate(numeric_effect, state_fluent_numeric_variables, state_auxiliary_numeric_variables);
            }
        }
    }
}

std::pair<State, const FlatDoubleList*>
StateRepository::get_or_create_successor_state(DenseState& dense_state, GroundAction action, const FlatDoubleList& auxiliary_numeric_state_variables)
{
    auto& dense_fluent_atoms = dense_state.get_atoms<Fluent>();
    auto& dense_derived_atoms = dense_state.get_atoms<Derived>();
    auto& dense_fluent_numeric_variables = dense_state.get_numeric_variables();
    m_state_auxiliary_numeric_variables = auxiliary_numeric_state_variables;

    auto& state_fluent_atoms_ptr = m_state_builder.get_fluent_atoms();
    auto& state_derived_atoms_ptr = m_state_builder.get_derived_atoms();
    auto& state_fluent_numeric_variables_ptr = m_state_builder.get_numeric_variables();
    m_state_fluent_atoms.clear();
    m_state_derived_atoms.clear();
    state_fluent_atoms_ptr = nullptr;
    state_derived_atoms_ptr = nullptr;

    /* 1. Set the state index. */
    {
        auto& state_index = m_state_builder.get_index();
        const auto next_state_index = Index(m_states.size());
        state_index = next_state_index;
    }

    /* 2. Apply action effects to construct non-extended state. */
    m_applied_negative_effect_atoms.unset_all();
    m_applied_positive_effect_atoms.unset_all();

    // TODO(numeric): for the beginning we just apply the numeric effects directly instead of collecting them...
    collect_applied_conjunctive_effects(action,
                                        dense_fluent_numeric_variables,
                                        m_state_auxiliary_numeric_variables,
                                        m_applied_negative_effect_atoms,
                                        m_applied_positive_effect_atoms,
                                        dense_fluent_numeric_variables,
                                        m_state_auxiliary_numeric_variables);

    collect_applied_conditional_effects(action,
                                        dense_fluent_numeric_variables,
                                        m_state_auxiliary_numeric_variables,
                                        m_axiom_evaluator->get_axiom_grounder()->get_problem(),
                                        dense_state,
                                        m_applied_negative_effect_atoms,
                                        m_applied_positive_effect_atoms,
                                        dense_fluent_numeric_variables,
                                        m_state_auxiliary_numeric_variables);

    apply_action_effects(m_applied_negative_effect_atoms, m_applied_positive_effect_atoms, dense_fluent_atoms);

    update_reached_fluent_atoms(dense_fluent_atoms, m_reached_fluent_atoms);

    translate_dense_into_sorted_compressed_sparse(dense_fluent_atoms, m_state_fluent_atoms);

    const auto [fluent_iter, fluent_inserted] = m_fluent_atoms_set.insert(m_state_fluent_atoms);
    update_state_atoms_ptr(**fluent_iter, state_fluent_atoms_ptr);

    const auto [fluent_numeric_iter, fluent_numeric_inserted] = m_fluent_numeric_variables_set.insert(dense_fluent_numeric_variables);
    update_state_numeric_variables_ptr(**fluent_numeric_iter, state_fluent_numeric_variables_ptr);

    const auto [auxiliary_numeric_iter, auxiliary_numeric_inserted] = m_auxiliary_numeric_variables_set.insert(m_state_auxiliary_numeric_variables);

    // Check if non-extended state exists in cache
    const auto state_iter = m_states.find(m_state_builder);
    if (state_iter != m_states.end())
    {
        return { state_iter->get(), auxiliary_numeric_iter->get() };
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

            const auto [iter, inserted] = m_derived_atoms_set.insert(m_state_derived_atoms);
            update_state_atoms_ptr(**iter, state_derived_atoms_ptr);
        }
    }

    // Cache and return the extended state.
    return { m_states.insert(m_state_builder).first->get(), auxiliary_numeric_iter->get() };
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
    return m_derived_atoms_set.get_estimated_memory_usage_in_bytes();
}
}
