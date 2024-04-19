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
 *<
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "mimir/search/applicable_action_generators/dense_grounded.hpp"

#include "mimir/common/printers.hpp"
#include "mimir/formalism/transformers/delete_relax.hpp"
#include "mimir/search/algorithms/brfs.hpp"
#include "mimir/search/applicable_action_generators/dense_lifted.hpp"
#include "mimir/search/applicable_action_generators/interface.hpp"
#include "mimir/search/event_handlers/minimal.hpp"
#include "mimir/search/successor_state_generators/dense.hpp"

namespace mimir
{

AAG<GroundedAAGDispatcher<DenseStateTag>>::AAG(Problem problem, PDDLFactories& pddl_factories) : m_problem(problem), m_pddl_factories(pddl_factories)
{
    // 1. Explore delete relaxed task.
    auto delete_relax_transformer = DeleteRelaxTransformer(m_pddl_factories);
    const auto delete_relaxed_problem = delete_relax_transformer.run(*m_problem);
    auto lifted_aag = AAG<LiftedAAGDispatcher<DenseStateTag>>(delete_relaxed_problem, m_pddl_factories);
    auto ssg = SSG<SSGDispatcher<DenseStateTag>>(delete_relaxed_problem);

    auto& state_bitset = m_state_builder.get_atoms_bitset();
    state_bitset.unset_all();

    // Keep track of changes
    size_t num_atoms = 0;
    size_t num_actions = 0;
    // Temporary variables
    auto actions = std::vector<ConstDenseActionViewProxy> {};
    do
    {
        num_atoms = pddl_factories.get_ground_atoms().size();
        num_actions = lifted_aag.get_actions().size();

        // Create a state where all ground atoms are true
        for (const auto& atom : pddl_factories.get_ground_atoms())
        {
            state_bitset.set(atom.get_identifier());
        }
        m_state_builder.get_flatmemory_builder().finish();
        const auto state = ConstDenseStateViewProxy(ConstDenseStateView(m_state_builder.get_flatmemory_builder().buffer().data()));

        // Create all applicable actions and apply newly generated actions
        actions.clear();
        lifted_aag.generate_applicable_actions(state, actions);
        for (const auto& action : actions)
        {
            const auto is_newly_generated = (action.get_id() >= num_actions);
            if (is_newly_generated)
            {
                (void) ssg.get_or_create_successor_state(state, action);
            }
        }

    } while (num_atoms != pddl_factories.get_ground_atoms().size());

    std::cout << "Total number of ground atoms reachable in delete-relaxed task: " << m_pddl_factories.get_ground_atoms().size() << std::endl;
    std::cout << "Total number of ground actions in delete-relaxed tasks: " << lifted_aag.get_actions().size() << std::endl;

    // TODO: 2. Build match tree
}

}
