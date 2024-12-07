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

#include "mimir/search/applicable_action_generators/lifted.hpp"

#include "mimir/common/itertools.hpp"
#include "mimir/formalism/action.hpp"
#include "mimir/formalism/literal.hpp"
#include "mimir/formalism/object.hpp"
#include "mimir/formalism/predicate_tag.hpp"
#include "mimir/formalism/repositories.hpp"
#include "mimir/formalism/utils.hpp"
#include "mimir/formalism/variable.hpp"
#include "mimir/search/action.hpp"
#include "mimir/search/condition_grounders.hpp"
#include "mimir/search/grounding/consistency_graph.hpp"

#include <boost/dynamic_bitset.hpp>
#include <stdexcept>
#include <vector>

using namespace std::string_literals;

namespace mimir
{

std::generator<GroundAction> LiftedApplicableActionGenerator::generate_applicable_actions(State state)
{
    // m_event_handler->on_start_generating_applicable_actions();

    // Create the assignment sets that are shared by all action schemas.

    const auto problem = m_grounder.get_problem();
    auto& pddl_repositories = m_grounder.get_pddl_repositories();

    auto& fluent_predicates = problem->get_domain()->get_predicates<Fluent>();
    auto fluent_atoms = pddl_repositories->get_ground_atoms_from_indices<Fluent>(state->get_atoms<Fluent>());

    auto fluent_assignment_set = AssignmentSet<Fluent>(problem, fluent_predicates, fluent_atoms);

    auto& derived_predicates = problem->get_problem_and_domain_derived_predicates();

    auto derived_atoms = pddl_repositories->get_ground_atoms_from_indices<Derived>(state->get_atoms<Derived>());
    auto derived_assignment_set = AssignmentSet<Derived>(problem, derived_predicates, derived_atoms);

    // Get all applicable ground actions.
    // This is done by getting bindings in the given state using the precondition.
    // These bindings are then used to ground the actual action schemas.

    for (auto& [action, condition_grounder] : m_grounder.get_action_precondition_grounders())
    {
        for (auto&& binding : condition_grounder.compute_bindings(state, fluent_assignment_set, derived_assignment_set))
        {
            try
            {
                co_yield m_grounder.ground_action(action, std::move(binding));
            }
            catch (const std::exception& e)
            {
                std::cerr << "Exception in sink: " << e.what() << std::endl;
            }
        }
    }

    // m_event_handler->on_end_generating_applicable_actions(out_applicable_actions, *m_pddl_repositories);
}

LiftedApplicableActionGenerator::LiftedApplicableActionGenerator(Problem problem, std::shared_ptr<PDDLRepositories> pddl_repositories) :
    m_grounder(problem, std::move(pddl_repositories))
{
}

ActionGrounder& LiftedApplicableActionGenerator::get_action_grounder() { return m_grounder; }
const ActionGrounder& LiftedApplicableActionGenerator::get_action_grounder() const { return m_grounder; }

}
