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
    DenseStateBuilder m_state_builder;

    // We need to unset all derived atoms before fixed point iteration
    FlatBitsetBuilder m_derived_atoms_bitset;

    /* Implement IStaticSSG interface */
    friend class IStaticSSG<SSG<SSGDispatcher<DenseStateTag>>>;

    [[nodiscard]] DenseState get_or_create_initial_state_impl(Problem problem)
    {
        int next_state_id = m_states.size();

        // Fetch member references.

        auto& state_id = m_state_builder.get_id();
        auto& state_bitset = m_state_builder.get_atoms_bitset();

        // Assign values to members.

        state_id = next_state_id;

        state_bitset.unset_all();

        for (const auto& literal : problem->get_initial_literals())
        {
            if (literal->is_negated())
            {
                throw std::runtime_error("negative literals in the initial state are not supported");
            }
            else
            {
                auto position = literal->get_atom()->get_identifier();
                state_bitset.set(position);
            }
        }

        /* Axioms */
        state_bitset -= m_derived_atoms_bitset;
        m_aag->generate_and_apply_axioms(state_bitset, m_derived_atoms_bitset);

        /* Construct the state and store it. */
        auto& flatmemory_builder = m_state_builder.get_flatmemory_builder();
        flatmemory_builder.finish();
        const auto [iter, inserted] = m_states.insert(flatmemory_builder);
        return DenseState(*iter);
    }

    [[nodiscard]] DenseState get_or_create_successor_state_impl(DenseState state, DenseAction action)
    {
        int next_state_id = m_states.size();

        // Fetch member references.

        auto& state_id = m_state_builder.get_id();
        auto& state_bitset = m_state_builder.get_atoms_bitset();

        // Assign values to members.
        state_id = next_state_id;
        state_bitset.unset_all();
        state_bitset |= state.get_atoms_bitset();

        /* Simple effects*/
        state_bitset -= action.get_unconditional_negative_effect_bitset();
        state_bitset |= action.get_unconditional_positive_effect_bitset();

        /* Conditional effects */
        const auto num_conditional_effects = action.get_conditional_effects().size();
        for (size_t i = 0; i < num_conditional_effects; ++i)
        {
            if (state.get_atoms_bitset().is_superseteq(action.get_conditional_positive_precondition_bitsets()[i])
                && state.get_atoms_bitset().are_disjoint(action.get_conditional_negative_precondition_bitsets()[i]))
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

        /* Axioms */
        state_bitset -= m_derived_atoms_bitset;

        m_aag->generate_and_apply_axioms(state_bitset, m_derived_atoms_bitset);

        /* Construct the state and store it. */
        auto& flatmemory_builder = m_state_builder.get_flatmemory_builder();
        flatmemory_builder.finish();
        const auto [iter, inserted] = m_states.insert(flatmemory_builder);
        return DenseState(*iter);
    }

    [[nodiscard]] size_t get_state_count_impl() const { return m_states.size(); }

public:
    explicit SSG(Problem problem, std::shared_ptr<IDynamicAAG> aag) : m_problem(problem), m_aag(std::move(aag)) {}
};

/**
 * Types
 */

using DenseSSG = SSG<SSGDispatcher<DenseStateTag>>;

}

#endif