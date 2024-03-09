#ifndef MIMIR_SEARCH_SUCCESSOR_STATE_GENERATORS_BITSET_HPP_
#define MIMIR_SEARCH_SUCCESSOR_STATE_GENERATORS_BITSET_HPP_

#include "interface.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/formalism/problem.hpp"
#include "mimir/search/config.hpp"

#include <cstddef>
#include <flatmemory/flatmemory.hpp>
#include <stdexcept>

namespace mimir
{

/**
 * Implementation class
 */
template<IsPlanningModeTag P>
class SSG<SSGDispatcher<P, BitsetStateTag>> : public ISSG<SSG<SSGDispatcher<P, BitsetStateTag>>>
{
private:
    using ConstStateView = ConstView<StateDispatcher<BitsetStateTag, P>>;
    using ConstActionView = ConstView<ActionDispatcher<P, BitsetStateTag>>;

    Problem m_problem;

    BitsetStateSet m_states;
    Builder<StateDispatcher<BitsetStateTag, P>> m_state_builder;

    /* Implement ISSG interface */
    template<typename>
    friend class ISSG;

    [[nodiscard]] ConstStateView get_or_create_initial_state_impl(const Problem& problem)
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

        // Construct the state and store it.

        auto& flatmemory_builder = m_state_builder.get_flatmemory_builder();
        flatmemory_builder.finish();
        return ConstStateView(m_states.insert(flatmemory_builder));
    }

    [[nodiscard]] ConstStateView get_or_create_successor_state_impl(ConstStateView state, ConstActionView action)
    {
        int next_state_id = m_states.size();

        // Fetch member references.

        auto& state_id = m_state_builder.get_id();
        auto& state_bitset = m_state_builder.get_atoms_bitset();

        // Assign values to members.

        state_id = next_state_id;

        state_bitset.unset_all();

        // TODO: Optimize this operation, this is quite inefficient.

        for (const auto& position : state.get_atoms_bitset())
        {
            state_bitset.set(position);
        }

        for (const auto& position : action.get_unconditional_negative_effect_bitset())
        {
            state_bitset.unset(position);
        }

        for (const auto& position : action.get_unconditional_positive_effect_bitset())
        {
            state_bitset.set(position);
        }

        // Construct the state and store it.

        auto& flatmemory_builder = m_state_builder.get_flatmemory_builder();
        flatmemory_builder.finish();
        return ConstStateView(m_states.insert(flatmemory_builder));
    }

    [[nodiscard]] size_t state_count_impl() const { return m_states.size(); }

public:
    explicit SSG(Problem problem) : m_problem(problem) {}
};

}

#endif
