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

#include "mimir/search/condition_grounders/conjunction_grounder.hpp"

#include "mimir/formalism/predicate_tag.hpp"
#include "mimir/formalism/problem.hpp"
#include "mimir/formalism/repositories.hpp"
#include "mimir/formalism/utils.hpp"
#include "mimir/search/applicable_action_generators/lifted/assignment_set.hpp"

#include <tuple>
#include <utility>

namespace mimir
{

LiftedConjunctionGrounder::LiftedConjunctionGrounder(Problem problem,
                                                     VariableList variables,
                                                     LiteralList<Static> static_literals,
                                                     LiteralList<Fluent> fluent_literals,
                                                     LiteralList<Derived> derived_literals,
                                                     std::shared_ptr<PDDLRepositories> pddl_repositories) :
    m_condition_grounder(std::move(problem),
                         std::move(variables),
                         std::move(static_literals),
                         std::move(fluent_literals),
                         std::move(derived_literals),
                         AssignmentSet<Static>(problem,  //
                                               problem->get_domain()->get_predicates<Static>(),
                                               to_ground_atoms(problem->get_static_initial_literals())),
                         pddl_repositories),
    m_pddl_repositories(pddl_repositories)
{
}

std::pair<std::vector<ObjectList>, std::tuple<std::vector<GroundLiteralList<Static>>, std::vector<GroundLiteralList<Fluent>>, std::vector<GroundLiteralList<Derived>>>> LiftedConjunctionGrounder::ground(State state)
{
    auto problem = m_condition_grounder.get_problem();

    auto& fluent_predicates = problem->get_domain()->get_predicates<Fluent>();
    auto fluent_atoms = m_pddl_repositories->get_ground_atoms_from_indices<Fluent>(state->get_atoms<Fluent>());
    auto fluent_assignment_set = AssignmentSet<Fluent>(problem, fluent_predicates, fluent_atoms);

    auto& derived_predicates = problem->get_problem_and_domain_derived_predicates();
    auto derived_atoms = m_pddl_repositories->get_ground_atoms_from_indices<Derived>(state->get_atoms<Derived>());
    auto derived_assignment_set = AssignmentSet<Derived>(problem, derived_predicates, derived_atoms);

    std::vector<ObjectList> bindings;
    std::vector<GroundLiteralList<Static>> static_grounded_literal_list;
    std::vector<GroundLiteralList<Fluent>> fluent_grounded_literal_list;
    std::vector<GroundLiteralList<Derived>> derived_grounded_literal_list;

    m_condition_grounder.compute_bindings(state, fluent_assignment_set, derived_assignment_set, bindings);

    for (const auto& binding : bindings)
    {
        GroundLiteralList<Static> static_grounded_literals;
        for (const auto& static_literal : m_condition_grounder.get_conditions<Static>())
        {
            static_grounded_literals.emplace_back(m_pddl_repositories->ground_literal(static_literal, binding));
        }
        static_grounded_literal_list.emplace_back(static_grounded_literals);

        GroundLiteralList<Fluent> fluent_grounded_literals;
        for (const auto& fluent_literal : m_condition_grounder.get_conditions<Fluent>())
        {
            fluent_grounded_literals.emplace_back(m_pddl_repositories->ground_literal(fluent_literal, binding));
        }
        fluent_grounded_literal_list.emplace_back(fluent_grounded_literals);

        GroundLiteralList<Derived> derived_grounded_literals;
        for (const auto& derived_literal : m_condition_grounder.get_conditions<Derived>())
        {
            derived_grounded_literals.emplace_back(m_pddl_repositories->ground_literal(derived_literal, binding));
        }
        derived_grounded_literal_list.emplace_back(derived_grounded_literals);
    }

    return std::make_pair(bindings, std::make_tuple(static_grounded_literal_list, fluent_grounded_literal_list, derived_grounded_literal_list));
}

}
