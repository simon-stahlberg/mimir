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
    Problem m_problem;
    std::shared_ptr<IDynamicAAG> m_aag;

    FlatDenseStateSet m_states;
    std::vector<DenseState> m_states_by_index;
    FlatDenseStateVector m_extended_states_by_state;
    DenseStateBuilder m_state_builder;

    /* Implement IStaticSSG interface */
    friend class IStaticSSG<SSG<SSGDispatcher<DenseStateTag>>>;

    [[nodiscard]] DenseState get_or_create_initial_state_impl(Problem problem)
    {
        int next_state_id = m_states.size();

        // Fetch member references.

        auto& state_id = m_state_builder.get_id();
        auto& state_bitset = m_state_builder.get_atoms_bitset();
        state_bitset.unset_all();

        /* 1. Set state id */

        state_id = next_state_id;

        /* 2. Construct non-extended state */

        for (const auto& literal : problem->get_initial_literals())
        {
            state_bitset.set(literal->get_atom()->get_identifier());
        }

        auto& flatmemory_builder = m_state_builder.get_flatmemory_builder();
        flatmemory_builder.finish();
        const auto [iter, inserted] = m_states.insert(flatmemory_builder);
        const auto state = DenseState(*iter);

        /* 3. Retrieve cached extended state */

        if (!inserted)
        {
            return DenseState(m_extended_states_by_state[state.get_id()]);
        }

        m_states_by_index.push_back(state);

        /* 4. Construct extended state by evaluating Axioms */

        m_aag->generate_and_apply_axioms(state_bitset);
        flatmemory_builder.finish();

        /* 5. Cache extended state */

        m_extended_states_by_state.push_back(flatmemory_builder);

        /* 6. Return newly generated extended state */

        return DenseState(m_extended_states_by_state[next_state_id]);
    }

    [[nodiscard]] DenseState get_or_create_successor_state_impl(DenseState state, DenseAction action)
    {
        int next_state_id = m_states.size();

        // Fetch member references.

        auto& state_id = m_state_builder.get_id();
        auto& state_bitset = m_state_builder.get_atoms_bitset();
        // TODO: add assignment operator to bitset to replace unset + operator|=
        state_bitset.unset_all();
        const auto& unextended_state = m_states_by_index[state.get_id()];
        state_bitset |= unextended_state.get_atoms_bitset();

        /* 1. Set state id */

        state_id = next_state_id;

        /* 2. Construct non-extended state */

        /* Simple effects*/
        state_bitset -= action.get_unconditional_negative_effect_bitset();
        state_bitset |= action.get_unconditional_positive_effect_bitset();

        /* Conditional effects */
        for (const auto& flat_conditional_effect : action.get_conditional_effects())
        {
            const auto conditional_effect = FlatConditionalEffectProxy(flat_conditional_effect);
            if (state.get_atoms_bitset().is_superseteq(conditional_effect.get_positive_precondition_bitset())
                && state.get_atoms_bitset().are_disjoint(conditional_effect.get_negative_precondition_bitset()))
            {
                if (conditional_effect.get_is_negated_effect())
                {
                    state_bitset.unset(conditional_effect.get_effect_atom_id());
                }
                else
                {
                    state_bitset.set(conditional_effect.get_effect_atom_id());
                }
            }
        }

        auto& flatmemory_builder = m_state_builder.get_flatmemory_builder();
        flatmemory_builder.finish();
        const auto [iter, inserted] = m_states.insert(flatmemory_builder);
        const auto succ_state = DenseState(*iter);

        /* 3. Retrieve cached extended state */

        if (!inserted)
        {
            return DenseState(m_extended_states_by_state[succ_state.get_id()]);
        }

        m_states_by_index.push_back(succ_state);

        /* 4. Construct extended state by evaluating Axioms */

        m_aag->generate_and_apply_axioms(state_bitset);
        flatmemory_builder.finish();

        /* 5. Cache extended state */

        m_extended_states_by_state.push_back(flatmemory_builder);

        /* 6. Return newly generated extended state */

        return DenseState(m_extended_states_by_state[next_state_id]);
    }

    [[nodiscard]] size_t get_state_count_impl() const { return m_states.size(); }

public:
    explicit SSG(Problem problem, std::shared_ptr<IDynamicAAG> aag) :
        m_problem(problem),
        m_aag(std::move(aag)),
        m_states_by_index(),
        m_extended_states_by_state()
    {
        /* Error checking */
        for (const auto& literal : problem->get_initial_literals())
        {
            if (literal->is_negated())
            {
                throw std::runtime_error("negative literals in the initial state are not supported");
            }
        }
    }
};

/**
 * Types
 */

using DenseSSG = SSG<SSGDispatcher<DenseStateTag>>;

}

#endif