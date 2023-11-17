/*
 * Copyright (C) 2023 Simon Stahlberg
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

#include "../../include/mimir/generators/grounded_successor_generator.hpp"
#include "../../include/mimir/generators/lifted_successor_generator.hpp"
#include "../../include/mimir/generators/successor_generator_factory.hpp"

namespace mimir::planners
{
    bool compute_relaxed_reachable_actions(const std::chrono::high_resolution_clock::time_point end_time,
                                           const mimir::formalism::ProblemDescription& problem,
                                           mimir::formalism::ActionList& out_actions)
    {
        const auto relaxed_domain = relax(problem->domain, true, true);
        const auto relaxed_problem =
            mimir::formalism::create_problem(problem->name, relaxed_domain, problem->objects, problem->initial, problem->goal, problem->atom_costs);
        const mimir::planners::LiftedSuccessorGenerator successor_generator(relaxed_problem);

        std::equal_to<mimir::formalism::State> equals;
        mimir::formalism::ActionList relaxed_actions;
        mimir::formalism::State state = mimir::formalism::create_state(relaxed_problem->initial, relaxed_problem);

        while (true)
        {
            relaxed_actions.clear();
            auto next_state = state;

            if (!successor_generator.get_applicable_actions(end_time, next_state, relaxed_actions))
            {
                return false;
            }

            for (const auto& action : relaxed_actions)
            {
                // Since there are no negative preconditions and negative effects, all actions are still applicable in the resulting state.
                next_state = mimir::formalism::apply(action, next_state);
            }

            if (equals(state, next_state))
            {
                break;
            }

            state = next_state;
        }

        // TODO: We can likely optimize the mapping between the relaxed action schemas and the original action schemas

        std::map<std::string, mimir::formalism::ActionSchema> action_schemas;

        for (const auto& action_schema : problem->domain->action_schemas)
        {
            action_schemas.emplace(action_schema->name, action_schema);
        }

        const auto& static_predicates = problem->domain->static_predicates;
        const auto& static_atoms = problem->get_static_atoms();

        const auto is_statically_inapplicable = [&static_predicates, &static_atoms](const mimir::formalism::Action& ground_action)
        {
            for (const auto& literal : ground_action->get_precondition())
            {
                const auto is_static_predicate = std::count(static_predicates.begin(), static_predicates.end(), literal->atom->predicate);

                if (is_static_predicate && (static_atoms.contains(literal->atom) == literal->negated))
                {
                    return true;
                }
            }

            return false;
        };

        // Make sure that each atom in the initial state and goal has a rank, they don't necessarily have to be mentioned in a ground action

        for (const auto& atom : problem->initial)
        {
            problem->get_rank(atom);
        }

        for (const auto& literal : problem->goal)
        {
            problem->get_rank(literal->atom);
        }

        // Create all ground actions for the original problem

        for (const auto& relaxed_action : relaxed_actions)
        {
            auto action_schema = action_schemas.at(relaxed_action->schema->name);
            auto arguments = relaxed_action->get_arguments();
            auto cost = relaxed_action->cost;

            const auto ground_action = mimir::formalism::create_action(problem, action_schema, std::move(arguments), cost);

            if (!is_statically_inapplicable(ground_action))
            {
                out_actions.emplace_back(ground_action);
            }
        }

        return true;
    }

    SuccessorGenerator create_sucessor_generator(const mimir::formalism::ProblemDescription& problem, SuccessorGeneratorType type)
    {
        switch (type)
        {
            case SuccessorGeneratorType::AUTOMATIC:
            {
                auto time_start = std::chrono::high_resolution_clock::now();
                auto time_end = time_start + std::chrono::seconds(60);
                mimir::formalism::ActionList actions;

                if (compute_relaxed_reachable_actions(time_end, problem, actions))
                {
                    return std::make_shared<GroundedSuccessorGenerator>(problem, actions);
                }

                return std::make_shared<LiftedSuccessorGenerator>(problem);
            }

            case SuccessorGeneratorType::LIFTED:
            {
                return std::make_shared<LiftedSuccessorGenerator>(problem);
            }

            case SuccessorGeneratorType::GROUNDED:
            {
                // The lifted successor generator can overapproximate all actions applicable in reachable states by only taking static atoms into
                // considerations. The grounded successor generator is a decision tree structure over these actions.
                const auto time_max = std::chrono::high_resolution_clock::time_point::max();
                mimir::formalism::ActionList actions;
                compute_relaxed_reachable_actions(time_max, problem, actions);
                return std::make_shared<GroundedSuccessorGenerator>(problem, actions);
            }

            default:
            {
                throw std::runtime_error("successor generator type is not yet implemented");
            }
        }
    }
}  // namespace planners
