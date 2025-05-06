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
#include "mimir/formalism/problem.hpp"
#include "mimir/formalism/repositories.hpp"
#include "mimir/search/applicability.hpp"
#include "mimir/search/axiom_evaluators/interface.hpp"
#include "mimir/search/search_context.hpp"

using namespace mimir::formalism;

namespace mimir::search
{

ContinuousCost compute_state_metric_value(State state, const ProblemImpl& problem)
{
    if (problem.get_auxiliary_function_value().has_value())
    {
        return problem.get_auxiliary_function_value().value()->get_number();
    }

    return problem.get_optimization_metric().has_value() ? evaluate(problem.get_optimization_metric().value()->get_function_expression(),
                                                                    problem.get_initial_function_to_value<StaticTag>(),
                                                                    state->get_numeric_variables()) :
                                                           0.;
}

StateRepositoryImpl::StateRepositoryImpl(AxiomEvaluator axiom_evaluator) :
    m_axiom_evaluator(std::move(axiom_evaluator)),
    m_problem_or_domain_has_axioms(!m_axiom_evaluator->get_problem()->get_problem_and_domain_axioms().empty()),
    m_states(),
    m_reached_fluent_atoms(),
    m_reached_derived_atoms(),
    m_dense_state_builder(),
    m_applied_positive_effect_atoms(),
    m_applied_negative_effect_atoms(),
    m_state_fluent_atoms(),
    m_state_derived_atoms(),
    m_empty_index_list(),
    m_empty_double_list()
{
    m_empty_index_list.compress();
}

StateRepository StateRepositoryImpl::create(AxiomEvaluator axiom_evaluator) { return std::make_shared<StateRepositoryImpl>(axiom_evaluator); }

std::pair<State, ContinuousCost> StateRepositoryImpl::get_or_create_initial_state()
{
    const auto problem = m_axiom_evaluator->get_problem();
    return get_or_create_state(problem->get_fluent_initial_atoms(), problem->get_initial_function_to_value<FluentTag>());
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

std::pair<State, ContinuousCost> StateRepositoryImpl::get_or_create_state(const GroundAtomList<FluentTag>& atoms,
                                                                          const FlatDoubleList& fluent_numeric_variables)
{
    auto& problem = *m_axiom_evaluator->get_problem();

    /* Dense state */
    auto& dense_fluent_atoms = m_dense_state_builder.get_atoms<FluentTag>();
    dense_fluent_atoms.unset_all();
    auto& dense_derived_atoms = m_dense_state_builder.get_atoms<DerivedTag>();
    dense_derived_atoms.unset_all();
    auto& dense_fluent_numeric_variables = m_dense_state_builder.get_numeric_variables();
    /* Temporaries */
    m_state_fluent_atoms.clear();
    m_state_derived_atoms.clear();
    /* Sparse state */
    const FlatIndexList* state_fluent_atoms = &m_empty_index_list;
    const FlatIndexList* state_derived_atoms = &m_empty_index_list;
    const FlatDoubleList* state_numeric_variables = &m_empty_double_list;

    /* 2. Construct non-extended state */

    /* 2.1 Numeric state variables */
    dense_fluent_numeric_variables = fluent_numeric_variables;

    state_numeric_variables = problem.get_or_create_double_list(dense_fluent_numeric_variables);

    /* 2.2. Propositional state */
    for (const auto& atom : atoms)
    {
        dense_fluent_atoms.set(atom->get_index());
    }
    update_reached_fluent_atoms(dense_fluent_atoms, m_reached_fluent_atoms);
    translate_dense_into_sorted_compressed_sparse(dense_fluent_atoms, m_state_fluent_atoms);
    assert(std::equal(m_state_fluent_atoms.compressed_begin(), m_state_fluent_atoms.compressed_end(), dense_fluent_atoms.begin()));
    assert(std::equal(dense_fluent_atoms.begin(), dense_fluent_atoms.end(), m_state_fluent_atoms.compressed_begin()));
    state_fluent_atoms = problem.get_or_create_index_list(m_state_fluent_atoms);

    // Test whether there exists an extended state for the given non extended state
    if (auto state = m_states.find(StateImpl(-1, state_fluent_atoms, state_derived_atoms, state_numeric_variables)))
    {
        return { state, compute_state_metric_value(state, *m_axiom_evaluator->get_problem()) };
    }

    /* 3. Apply axioms to construct extended state. */
    {
        if (m_problem_or_domain_has_axioms)
        {
            // Evaluate axioms
            m_axiom_evaluator->generate_and_apply_axioms(m_dense_state_builder);
            update_reached_derived_atoms(dense_derived_atoms, m_reached_derived_atoms);
            translate_dense_into_sorted_compressed_sparse(dense_derived_atoms, m_state_derived_atoms);
            assert(std::equal(m_state_derived_atoms.compressed_begin(), m_state_derived_atoms.compressed_end(), dense_derived_atoms.begin()));
            assert(std::equal(dense_derived_atoms.begin(), dense_derived_atoms.end(), m_state_derived_atoms.compressed_begin()));
            state_derived_atoms = problem.get_or_create_index_list(m_state_derived_atoms);
        }
    }

    // Cache and return the extended state.
    auto state = m_states.get_or_create(state_fluent_atoms, state_derived_atoms, state_numeric_variables);

    return { state, compute_state_metric_value(state, *m_axiom_evaluator->get_problem()) };
}

std::pair<State, ContinuousCost> StateRepositoryImpl::get_or_create_successor_state(State state, GroundAction action, ContinuousCost state_metric_value)
{
    DenseState::translate(state, m_dense_state_builder);

    return get_or_create_successor_state(state, m_dense_state_builder, action, state_metric_value);
}

static void apply_numeric_effect(const std::pair<loki::AssignOperatorEnum, ContinuousCost>& numeric_effect, ContinuousCost& ref_value)
{
    const auto [assign_operator, value] = numeric_effect;

    assert(assign_operator == loki::AssignOperatorEnum::ASSIGN || ref_value != UNDEFINED_CONTINUOUS_COST);

    switch (assign_operator)
    {
        case loki::AssignOperatorEnum::ASSIGN:
        {
            ref_value = value;
            break;
        }
        case loki::AssignOperatorEnum::INCREASE:
        {
            ref_value += value;
            break;
        }
        case loki::AssignOperatorEnum::DECREASE:
        {
            ref_value -= value;
            break;
        }
        case loki::AssignOperatorEnum::SCALE_UP:
        {
            ref_value *= value;
            break;
        }
        case loki::AssignOperatorEnum::SCALE_DOWN:
        {
            assert(value != 0);
            ref_value /= value;
            break;
        }
        default:
        {
            throw std::logic_error("apply_numeric_effect(numeric_effect, ref_value): Unexpected loki::AssignOperatorEnum.");
        }
    }
}

static void collect_applied_fluent_numeric_effects(const GroundNumericEffectList<FluentTag>& numeric_effects,
                                                   const FlatDoubleList& static_numeric_variables,
                                                   const FlatDoubleList& fluent_numeric_variables,
                                                   FlatDoubleList& ref_numeric_variables)
{
    assert(&fluent_numeric_variables != &ref_numeric_variables);

    for (const auto& numeric_effect : numeric_effects)
    {
        const auto index = numeric_effect->get_function()->get_index();
        if (index >= ref_numeric_variables.size())
        {
            ref_numeric_variables.resize(index + 1, UNDEFINED_CONTINUOUS_COST);
        }
        const auto assign_operator_and_value = evaluate(numeric_effect, static_numeric_variables, fluent_numeric_variables);

        apply_numeric_effect(assign_operator_and_value, ref_numeric_variables[index]);
    }
}

static void collect_applied_auxiliary_numeric_effects(const GroundNumericEffect<AuxiliaryTag>& numeric_effect,
                                                      const FlatDoubleList& static_numeric_variables,
                                                      const FlatDoubleList& fluent_numeric_variables,
                                                      ContinuousCost& ref_successor_state_metric_score)
{
    const auto assign_operator_and_value = evaluate(numeric_effect, static_numeric_variables, fluent_numeric_variables);
    assert(assign_operator_and_value.second != UNDEFINED_CONTINUOUS_COST);

    apply_numeric_effect(assign_operator_and_value, ref_successor_state_metric_score);
}

static void apply_action_effects(GroundAction action,
                                 const ProblemImpl& problem,
                                 State state,
                                 const DenseState& dense_state,
                                 FlatBitset& ref_dense_fluent_atoms,
                                 FlatBitset& ref_negative_applied_effects,
                                 FlatBitset& ref_positive_applied_effects,
                                 FlatDoubleList& ref_fluent_numeric_variables,
                                 ContinuousCost& ref_successor_state_metric_score)
{
    const auto& conjunctive_effect = action->get_conjunctive_effect();
    const auto& const_fluent_numeric_variables = state->get_numeric_variables();
    const auto& const_static_numeric_variables = problem.get_initial_function_to_value<StaticTag>();

    insert_into_bitset(conjunctive_effect->get_negative_effects(), ref_negative_applied_effects);
    insert_into_bitset(conjunctive_effect->get_positive_effects(), ref_positive_applied_effects);

    collect_applied_fluent_numeric_effects(conjunctive_effect->get_fluent_numeric_effects(),
                                           const_static_numeric_variables,
                                           const_fluent_numeric_variables,
                                           ref_fluent_numeric_variables);
    if (conjunctive_effect->get_auxiliary_numeric_effect().has_value())
    {
        collect_applied_auxiliary_numeric_effects(conjunctive_effect->get_auxiliary_numeric_effect().value(),
                                                  const_static_numeric_variables,
                                                  const_fluent_numeric_variables,
                                                  ref_successor_state_metric_score);
    }

    for (const auto& conditional_effect : action->get_conditional_effects())
    {
        if (is_applicable(conditional_effect, problem, dense_state))
        {
            insert_into_bitset(conditional_effect->get_conjunctive_effect()->get_negative_effects(), ref_negative_applied_effects);
            insert_into_bitset(conditional_effect->get_conjunctive_effect()->get_positive_effects(), ref_positive_applied_effects);
            collect_applied_fluent_numeric_effects(conditional_effect->get_conjunctive_effect()->get_fluent_numeric_effects(),
                                                   const_static_numeric_variables,
                                                   const_fluent_numeric_variables,
                                                   ref_fluent_numeric_variables);
            if (conditional_effect->get_conjunctive_effect()->get_auxiliary_numeric_effect().has_value())
            {
                collect_applied_auxiliary_numeric_effects(conditional_effect->get_conjunctive_effect()->get_auxiliary_numeric_effect().value(),
                                                          const_static_numeric_variables,
                                                          const_fluent_numeric_variables,
                                                          ref_successor_state_metric_score);
            }
        }
    }

    // Update propositional state atoms.
    ref_dense_fluent_atoms -= ref_negative_applied_effects;
    ref_dense_fluent_atoms |= ref_positive_applied_effects;

    // Update metric in case of a fluent one.
    if (!problem.get_domain()->get_auxiliary_function_skeleton().has_value())
    {
        ref_successor_state_metric_score =
            problem.get_optimization_metric().has_value() ?
                evaluate(problem.get_optimization_metric().value()->get_function_expression(), const_static_numeric_variables, ref_fluent_numeric_variables) :
                ref_successor_state_metric_score + 1;
    }
}

std::pair<State, ContinuousCost>
StateRepositoryImpl::get_or_create_successor_state(State state, DenseState& dense_state, GroundAction action, ContinuousCost state_metric_value)
{
    auto& problem = *m_axiom_evaluator->get_problem();

    /* Dense state*/
    auto& dense_fluent_atoms = dense_state.get_atoms<FluentTag>();
    m_state_fluent_atoms.clear();
    auto& dense_derived_atoms = dense_state.get_atoms<DerivedTag>();
    m_state_derived_atoms.clear();
    auto& dense_fluent_numeric_variables = dense_state.get_numeric_variables();
    /* Temporaries */
    m_state_fluent_atoms.clear();
    m_state_derived_atoms.clear();
    m_applied_negative_effect_atoms.unset_all();
    m_applied_positive_effect_atoms.unset_all();
    /* Sparse state */
    const FlatIndexList* state_fluent_atoms = &m_empty_index_list;
    const FlatIndexList* state_derived_atoms = &m_empty_index_list;
    const FlatDoubleList* state_numeric_variables = &m_empty_double_list;

    auto successor_state_metric_value = state_metric_value;

    /* 2. Apply action effects to construct non-extended state. */

    apply_action_effects(action,
                         problem,
                         state,
                         dense_state,
                         dense_fluent_atoms,
                         m_applied_negative_effect_atoms,
                         m_applied_positive_effect_atoms,
                         dense_fluent_numeric_variables,
                         successor_state_metric_value);
    update_reached_fluent_atoms(dense_fluent_atoms, m_reached_fluent_atoms);
    translate_dense_into_sorted_compressed_sparse(dense_fluent_atoms, m_state_fluent_atoms);
    if (!std::equal(m_state_fluent_atoms.compressed_begin(), m_state_fluent_atoms.compressed_end(), dense_fluent_atoms.begin()))
    {
        mimir::operator<<(std::cout, dense_fluent_atoms);
        std::cout << std::endl;
        mimir::operator<<(std::cout, m_state_fluent_atoms);
        std::cout << std::endl;
    }
    assert(std::equal(m_state_fluent_atoms.compressed_begin(), m_state_fluent_atoms.compressed_end(), dense_fluent_atoms.begin()));
    assert(std::equal(dense_fluent_atoms.begin(), dense_fluent_atoms.end(), m_state_fluent_atoms.compressed_begin()));
    state_fluent_atoms = problem.get_or_create_index_list(m_state_fluent_atoms);
    state_numeric_variables = problem.get_or_create_double_list(dense_fluent_numeric_variables);

    // Check if non-extended state exists in cache
    if (auto state = m_states.find(StateImpl(-1, state_fluent_atoms, state_derived_atoms, state_numeric_variables)))
    {
        return { state, successor_state_metric_value };
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
            assert(std::equal(m_state_derived_atoms.compressed_begin(), m_state_derived_atoms.compressed_end(), dense_derived_atoms.begin()));
            assert(std::equal(dense_derived_atoms.begin(), dense_derived_atoms.end(), m_state_derived_atoms.compressed_begin()));
            state_derived_atoms = problem.get_or_create_index_list(m_state_derived_atoms);
        }
    }

    // Cache and return the extended state.
    return { m_states.get_or_create(state_fluent_atoms, state_derived_atoms, state_numeric_variables), successor_state_metric_value };
}

const Problem& StateRepositoryImpl::get_problem() const { return m_axiom_evaluator->get_problem(); }

size_t StateRepositoryImpl::get_state_count() const { return m_states.size(); }

const FlatBitset& StateRepositoryImpl::get_reached_fluent_ground_atoms_bitset() const { return m_reached_fluent_atoms; }

const FlatBitset& StateRepositoryImpl::get_reached_derived_ground_atoms_bitset() const { return m_reached_derived_atoms; }

const AxiomEvaluator& StateRepositoryImpl::get_axiom_evaluator() const { return m_axiom_evaluator; }

}
