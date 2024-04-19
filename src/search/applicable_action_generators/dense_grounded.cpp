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
    // Explore delete relaxed task.
    // TODO: improve performance by doing fixed point iteration instead
    auto delete_relax_transformer = DeleteRelaxTransformer(m_pddl_factories);
    const auto delete_relaxed_problem = delete_relax_transformer.run(*m_problem);
    auto state_repository = std::make_shared<SSG<SSGDispatcher<DenseStateTag>>>(delete_relaxed_problem);
    auto successor_generator = std::make_shared<AAG<LiftedAAGDispatcher<DenseStateTag>>>(delete_relaxed_problem, m_pddl_factories);
    auto event_handler = std::make_shared<MinimalEventHandler>();
    auto grounded_brfs = BrFsAlgorithm(delete_relaxed_problem, m_pddl_factories, state_repository, successor_generator, event_handler);
    auto plan = std::vector<ConstView<ActionDispatcher<StateReprTag>>> {};
    const auto search_status = grounded_brfs.find_solution(plan);

    std::cout << "Total number of atoms reachable in delete-relaxed task: " << m_pddl_factories.get_atoms().size() << std::endl;
    std::cout << "Total number of actions in delete-relaxed tasks: " << successor_generator->get_actions().size() << std::endl;

    // TODO: Build match tree
}

}
