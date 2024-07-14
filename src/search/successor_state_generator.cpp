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

#include "mimir/search/successor_state_generator.hpp"

#include "mimir/formalism/ground_literal.hpp"
#include "mimir/formalism/problem.hpp"
#include "mimir/search/applicable_action_generators.hpp"

namespace mimir
{
SuccessorStateGenerator::SuccessorStateGenerator(std::shared_ptr<IApplicableActionGenerator> aag) : m_aag(std::move(aag)) {}

State SuccessorStateGenerator::get_or_create_initial_state()
{
    auto ground_atoms = GroundAtomList<Fluent> {};

    for (const auto& literal : m_aag->get_problem()->get_fluent_initial_literals())
    {
        if (literal->is_negated())
        {
            throw std::runtime_error("negative literals in the initial state are not supported");
        }

        ground_atoms.push_back(literal->get_atom());
    }

    return get_or_create_state(ground_atoms);
}

State SuccessorStateGenerator::get_or_create_state(const GroundAtomList<Fluent>& atoms)
{
    /* Fetch member references for non extended construction. */

    auto& flatmemory_builder = m_state_builder.get_flatmemory_builder();
    auto& state_id = m_state_builder.get_id();
    auto& fluent_state_atoms = m_state_builder.get_atoms<Fluent>();
    fluent_state_atoms.unset_all();

    /* 1. Set state id */

    int next_state_id = m_states.size();
    state_id = next_state_id;

    /* 2. Construct non-extended state */

    for (const auto& atom : atoms)
    {
        fluent_state_atoms.set(atom->get_identifier());
    }
    m_reached_fluent_atoms |= fluent_state_atoms;

    /* 3. Retrieve cached extended state */

    // Test whether there exists an extended state for the given non extended state
    flatmemory_builder.finish();
    auto iter = m_states.find(FlatState(flatmemory_builder.buffer().data()));
    if (iter != m_states.end())
    {
        return State(*iter);
    }

    /* Fetch member references for extended construction. */

    auto& derived_state_atoms = m_state_builder.get_atoms<Derived>();
    derived_state_atoms.unset_all();

    /* 4. Construct extended state by evaluating Axioms */

    m_aag->generate_and_apply_axioms(fluent_state_atoms, derived_state_atoms);
    m_reached_derived_atoms |= derived_state_atoms;

    /* 5. Cache extended state */

    flatmemory_builder.finish();
    auto [iter2, inserted] = m_states.insert(flatmemory_builder);

    /* 6. Return newly generated extended state */

    return State(*iter2);
}

State SuccessorStateGenerator::get_or_create_successor_state(State state, GroundAction action)
{
    /* Fetch member references for non extended construction. */

    auto& flatmemory_builder = m_state_builder.get_flatmemory_builder();
    auto& state_id = m_state_builder.get_id();
    auto& fluent_state_atoms = m_state_builder.get_atoms<Fluent>();
    fluent_state_atoms.unset_all();

    // 1. Initialize non-extended state
    fluent_state_atoms = state.get_atoms<Fluent>();

    /* 2. Set state id */

    int next_state_id = m_states.size();
    state_id = next_state_id;

    /* 3. Construct non-extended state */

    /* STRIPS effects*/
    auto strips_part_proxy = StripsActionEffect(action.get_strips_effect());
    fluent_state_atoms -= strips_part_proxy.get_negative_effects();
    fluent_state_atoms |= strips_part_proxy.get_positive_effects();
    /* Conditional effects */
    for (const auto flat_conditional_effect : action.get_conditional_effects())
    {
        auto cond_effect_proxy = ConditionalEffect(flat_conditional_effect);

        if (cond_effect_proxy.is_applicable(m_aag->get_problem(), state))
        {
            const auto& simple_effect = cond_effect_proxy.get_simple_effect();

            if (simple_effect.is_negated)
            {
                fluent_state_atoms.unset(simple_effect.atom_id);
            }
            else
            {
                fluent_state_atoms.set(simple_effect.atom_id);
            }
        }
    }
    m_reached_fluent_atoms |= fluent_state_atoms;

    /* 4. Retrieve cached extended state */

    // Test whether there exists an extended state for the given non extended state
    flatmemory_builder.finish();
    auto iter = m_states.find(FlatState(flatmemory_builder.buffer().data()));
    if (iter != m_states.end())
    {
        return State(*iter);
    }

    /* Fetch member references for extended construction. */

    auto& derived_state_atoms = m_state_builder.get_atoms<Derived>();
    derived_state_atoms.unset_all();

    /* 5. Construct extended state by evaluating Axioms */

    m_aag->generate_and_apply_axioms(fluent_state_atoms, derived_state_atoms);
    m_reached_derived_atoms |= derived_state_atoms;

    /* 6. Cache extended state */

    flatmemory_builder.finish();
    auto [iter2, inserted] = m_states.insert(flatmemory_builder);

    /* 7. Return newly generated extended state */

    return State(*iter2);
}

size_t SuccessorStateGenerator::get_state_count() const { return m_states.size(); }

const FlatBitsetBuilder<Fluent>& SuccessorStateGenerator::get_reached_fluent_ground_atoms() const { return m_reached_fluent_atoms; }

const FlatBitsetBuilder<Derived>& SuccessorStateGenerator::get_reached_derived_ground_atoms() const { return m_reached_derived_atoms; }

std::shared_ptr<IAAG> SuccessorStateGenerator::get_aag() const { return m_aag; }
}
