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

#ifndef MIMIR_SEARCH_SUCCESSOR_STATE_GENERATORS_DENSE_HPP_
#define MIMIR_SEARCH_SUCCESSOR_STATE_GENERATORS_DENSE_HPP_

#include "mimir/common/collections.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/formalism/problem.hpp"
#include "mimir/search/applicable_action_generators.hpp"
#include "mimir/search/states/dense.hpp"
#include "mimir/search/successor_state_generators/interface.hpp"

#include <cstddef>
#include <flatmemory/flatmemory.hpp>
#include <stdexcept>

namespace mimir
{

/**
 * Implementation class
 */
template<>
class SSG<SSGDispatcher<DenseStateTag>> : public IStaticSSG<SSG<SSGDispatcher<DenseStateTag>>>
{
private:
    std::shared_ptr<IDynamicAAG> m_aag;

    // TODO: Is it possible to separate ground atoms over simple and derived predicates
    // to store extended and non-extended states more efficiently?
    FlatDenseStateSet m_states;
    std::vector<DenseState> m_states_by_index;
    FlatDenseStateVector m_extended_states_by_state;
    DenseStateBuilder m_state_builder;

    /* Implement IStaticSSG interface */
    friend class IStaticSSG<SSG<SSGDispatcher<DenseStateTag>>>;

    [[nodiscard]] DenseState get_or_create_initial_state_impl()
    {
        auto ground_atoms = GroundAtomList {};

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

    [[nodiscard]] State get_or_create_state_impl(const GroundAtomList& atoms)
    {
        /* Header */

        int next_state_id = m_states.size();

        /* Fetch member references. */

        auto& state_id = m_state_builder.get_id();
        auto& state_bitset = m_state_builder.get_atoms_bitset();
        state_bitset.unset_all();
        auto& problem = m_state_builder.get_problem();

        /* 1. Set state id */

        state_id = next_state_id;

        /* 2. Set problem */

        problem = m_aag->get_problem();

        /* 3. Construct non-extended state */

        for (const auto& atom : atoms)
        {
            state_bitset.set(atom->get_identifier());
        }

        auto& flatmemory_builder = m_state_builder.get_flatmemory_builder();
        flatmemory_builder.finish();
        const auto [iter, inserted] = m_states.insert(flatmemory_builder);
        const auto state = DenseState(*iter);

        /* 4. Retrieve cached extended state */

        if (!inserted)
        {
            return DenseState(m_extended_states_by_state[state.get_id()]);
        }

        m_states_by_index.push_back(state);

        /* 5. Construct extended state by evaluating Axioms */

        m_aag->generate_and_apply_axioms(state_bitset);
        flatmemory_builder.finish();

        /* 6. Cache extended state */

        m_extended_states_by_state.push_back(flatmemory_builder);

        /* 7. Return newly generated extended state */

        return DenseState(m_extended_states_by_state[next_state_id]);
    }

    [[nodiscard]] DenseState get_or_create_successor_state_impl(const DenseState state, const DenseGroundAction action)
    {
        int next_state_id = m_states.size();

        // Fetch member references.

        auto& state_id = m_state_builder.get_id();
        auto& state_bitset = m_state_builder.get_atoms_bitset();
        auto& problem = m_state_builder.get_problem();

        // TODO: add assignment operator to bitset to replace unset + operator|=
        state_bitset.unset_all();
        const auto& unextended_state = m_states_by_index[state.get_id()];
        state_bitset |= unextended_state.get_atoms_bitset();

        /* 1. Set state id */

        state_id = next_state_id;

        /* 2. Set problem */

        problem = m_aag->get_problem();

        /* 3. Construct non-extended state */

        /* Simple effects*/
        state_bitset -= action.get_unconditional_negative_effect_bitset();
        state_bitset |= action.get_unconditional_positive_effect_bitset();

        const auto positive_static_precondition = state.get_problem()->get_static_initial_positive_atoms_bitset();
        const auto negative_static_precondition = state.get_problem()->get_static_initial_negative_atoms_bitset();

        /* Conditional effects */
        const auto num_conditional_effects = action.get_conditional_effects().size();
        for (size_t i = 0; i < num_conditional_effects; ++i)
        {
            if (state.get_atoms_bitset().is_superseteq(action.get_conditional_positive_precondition_bitsets()[i])
                && positive_static_precondition.is_superseteq(action.get_conditional_positive_static_precondition_bitsets()[i])
                && state.get_atoms_bitset().are_disjoint(action.get_conditional_negative_precondition_bitsets()[i])
                && negative_static_precondition.is_superseteq(action.get_conditional_negative_static_precondition_bitsets()[i]))
            {
                const auto simple_effect = action.get_conditional_effects()[i];
                if (simple_effect.is_negated)
                {
                    state_bitset.unset(simple_effect.atom_id);
                }
                else
                {
                    state_bitset.set(simple_effect.atom_id);
                }
            }
        }

        auto& flatmemory_builder = m_state_builder.get_flatmemory_builder();
        flatmemory_builder.finish();
        const auto [iter, inserted] = m_states.insert(flatmemory_builder);
        const auto succ_state = DenseState(*iter);

        /* 4. Retrieve cached extended state */

        if (!inserted)
        {
            return DenseState(m_extended_states_by_state[succ_state.get_id()]);
        }

        m_states_by_index.push_back(succ_state);

        /* 5. Construct extended state by evaluating Axioms */

        m_aag->generate_and_apply_axioms(state_bitset);
        flatmemory_builder.finish();

        /* 6. Cache extended state */

        m_extended_states_by_state.push_back(flatmemory_builder);

        /* 7. Return newly generated extended state */

        return DenseState(m_extended_states_by_state[next_state_id]);
    }

    [[nodiscard]] DenseState get_non_extended_state_impl(const DenseState state)
    {  //
        return m_states_by_index[state.get_id()];
    }

    [[nodiscard]] size_t get_state_count_impl() const { return m_states.size(); }

public:
    explicit SSG(std::shared_ptr<IDynamicAAG> aag) : m_aag(std::move(aag)), m_states_by_index(), m_extended_states_by_state() {}
};

/**
 * Types
 */

using DenseSSG = SSG<SSGDispatcher<DenseStateTag>>;

}

#endif