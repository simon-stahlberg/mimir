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


#include "lifted_successor_generator.hpp"

#include <chrono>

namespace planners
{
    LiftedSuccessorGenerator::LiftedSuccessorGenerator(const formalism::DomainDescription& domain, const formalism::ProblemDescription& problem) :
        problem_(problem),
        generators_()
    {
        for (const auto& action_schema : domain->action_schemas)
        {
            generators_.insert(std::make_pair(action_schema, LiftedSchemaSuccessorGenerator(action_schema, problem)));
        }
    }

    formalism::ActionList LiftedSuccessorGenerator::get_applicable_actions(const formalism::State& state) const
    {
        formalism::ActionList applicable_actions;

        const auto assignment_sets = LiftedSchemaSuccessorGenerator::build_assignment_sets(problem_, state->get_dynamic_ranks());

        for (const auto& [_, generator] : generators_)
        {
            const auto schema_actions = generator.get_applicable_actions(state, assignment_sets);
            applicable_actions.insert(applicable_actions.end(), schema_actions.begin(), schema_actions.end());
        }

        return applicable_actions;
    }

    bool LiftedSuccessorGenerator::get_actions(int32_t timeout_s, formalism::ActionList& out_actions) const
    {
        formalism::ActionList actions;
        auto start_time = std::chrono::high_resolution_clock::now();
        auto end_time = start_time + std::chrono::seconds(timeout_s);

        for (const auto& [_, generator] : generators_)
        {
            if (std::chrono::high_resolution_clock::now() >= end_time)
            {
                return false;
            }

            if (!generator.get_actions(end_time, out_actions))
            {
                return false;
            }
        }

        return true;
    }

    formalism::ActionList LiftedSuccessorGenerator::get_actions() const
    {
        formalism::ActionList actions;

        for (const auto& [_, generator] : generators_)
        {
            const auto schema_actions = generator.get_actions();
            actions.insert(actions.end(), schema_actions.begin(), schema_actions.end());
        }

        return actions;
    }
}  // namespace planners
