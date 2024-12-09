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
#include "mimir/search/grounding/axiom_grounder.hpp"

namespace mimir
{
StateRepository::StateRepository(std::shared_ptr<IAxiomEvaluator> axiom_evaluator) :
    m_axiom_evaluator(std::move(axiom_evaluator)),
    m_problem_or_domain_has_axioms(!m_axiom_evaluator->get_axiom_grounder().get_problem()->get_axioms().empty()
                                   || !m_axiom_evaluator->get_axiom_grounder().get_problem()->get_domain()->get_axioms().empty()),
    m_states(),
    m_state_builder(),
    m_positive_applied_effects(),
    m_negative_applied_effects(),
    m_reached_fluent_atoms(),
    m_reached_derived_atoms()
{
}

State StateRepository::get_or_create_initial_state()
{
    auto ground_atoms = GroundAtomList<Fluent> {};

    for (const auto& literal : m_axiom_evaluator->get_axiom_grounder().get_problem()->get_fluent_initial_literals())
    {
        ground_atoms.push_back(literal->get_atom());
    }

    return get_or_create_state(ground_atoms);
}

State StateRepository::get_or_create_state(const GroundAtomList<Fluent>& atoms)
{
    /* 1. Set state index */

    {
        auto& state_index = m_state_builder.get_index();
        const auto next_state_index = Index(m_states.size());
        state_index = next_state_index;
    }

    /* 2. Construct non-extended state */

    {
        auto& fluent_state_atoms = m_state_builder.get_atoms<Fluent>();
        fluent_state_atoms.unset_all();
        for (const auto& atom : atoms)
        {
            fluent_state_atoms.set(atom->get_index());
        }
        m_reached_fluent_atoms |= fluent_state_atoms;

        // Test whether there exists an extended state for the given non extended state
        auto iter = m_states.find(m_state_builder);
        if (iter != m_states.end())
        {
            return *iter;
        }
    }

    /* 3. Apply axioms to construct extended state. */

    {
        // Return early if no axioms must be evaluated
        if (!m_problem_or_domain_has_axioms)
        {
            const auto [iter2, inserted] = m_states.insert(m_state_builder);
            return *iter2;
        }

        // Evaluate axioms
        auto& derived_state_atoms = m_state_builder.get_atoms<Derived>();
        derived_state_atoms.unset_all();
        m_axiom_evaluator->generate_and_apply_axioms(m_state_builder);

        // Update reached derived atoms
        m_reached_derived_atoms |= derived_state_atoms;
    }

    // Cache and return the extended state.
    return *m_states.insert(m_state_builder).first;
}

std::pair<State, ContinuousCost> StateRepository::get_or_create_successor_state(State state, GroundAction action)
{
    /* Accumulate state-dependent action cost. */
    auto action_cost = ContinuousCost(0);

    /* 1. Set the state index. */

    {
        auto& state_index = m_state_builder.get_index();
        const auto next_state_index = Index(m_states.size());
        state_index = next_state_index;
    }

    /* 2. Apply action effects to construct non-extended state. */

    {
        // Apply STRIPS effect
        const auto& strips_action_effect = action->get_strips_effect();
        m_negative_applied_effects = strips_action_effect.get_negative_effects();
        m_positive_applied_effects = strips_action_effect.get_positive_effects();
        action_cost += strips_action_effect.get_cost();

        // Apply conditional effects
        for (const auto& conditional_effect : action->get_conditional_effects())
        {
            if (conditional_effect.is_applicable(m_axiom_evaluator->get_axiom_grounder().get_problem(), state))
            {
                for (const auto& effect_literal : conditional_effect.get_fluent_effect_literals())
                {
                    if (effect_literal.is_negated)
                    {
                        m_negative_applied_effects.set(effect_literal.atom_index);
                    }
                    else
                    {
                        m_positive_applied_effects.set(effect_literal.atom_index);
                    }
                }
                action_cost += conditional_effect.get_cost();
            }
        }

        // Modify fluent state atoms
        auto& fluent_state_atoms = m_state_builder.get_atoms<Fluent>();
        fluent_state_atoms = state->get_atoms<Fluent>();
        fluent_state_atoms -= m_negative_applied_effects;
        fluent_state_atoms |= m_positive_applied_effects;

        // Update reached fluent atoms
        m_reached_fluent_atoms |= fluent_state_atoms;

        // Check if non-extended state exists in cache
        const auto iter = m_states.find(m_state_builder);
        if (iter != m_states.end())
        {
            return std::make_pair(*iter, action_cost);
        }
    }

    /* 3. Apply axioms to construct extended state. */

    {
        // Return early if no axioms must be evaluated
        if (!m_problem_or_domain_has_axioms)
        {
            const auto [iter2, inserted] = m_states.insert(m_state_builder);
            return std::make_pair(*iter2, action_cost);
        }

        // Evaluate axioms
        auto& derived_state_atoms = m_state_builder.get_atoms<Derived>();
        derived_state_atoms.unset_all();
        m_axiom_evaluator->generate_and_apply_axioms(m_state_builder);

        // Update reached derived atoms
        m_reached_derived_atoms |= derived_state_atoms;
    }

    // Cache and return the extended state.
    return std::make_pair(*m_states.insert(m_state_builder).first, action_cost);
}

size_t StateRepository::get_state_count() const { return m_states.size(); }

const FlatBitset& StateRepository::get_reached_fluent_ground_atoms_bitset() const { return m_reached_fluent_atoms; }

const FlatBitset& StateRepository::get_reached_derived_ground_atoms_bitset() const { return m_reached_derived_atoms; }

const std::shared_ptr<IAxiomEvaluator>& StateRepository::get_axiom_evaluator() const { return m_axiom_evaluator; }
}
