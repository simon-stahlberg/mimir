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

#include "mimir/search/state_repository.hpp"

#include "mimir/formalism/parser.hpp"
#include "mimir/formalism/repositories.hpp"
#include "mimir/search/applicable_action_generators.hpp"
#include "mimir/search/axiom_evaluators.hpp"
#include "mimir/search/metric.hpp"

#include <gtest/gtest.h>

namespace mimir::tests
{

TEST(MimirTests, SearchStateRepositoryTest)
{
    // Instantiate lifted version
    const auto domain_file = fs::path(std::string(DATA_DIR) + "gripper/domain.pddl");
    const auto problem_file = fs::path(std::string(DATA_DIR) + "gripper/test_problem.pddl");
    PDDLParser parser(domain_file, problem_file);
    auto applicable_action_generator = LiftedApplicableActionGenerator(parser.get_problem(), parser.get_pddl_repositories());
    const auto axiom_evaluator =
        std::dynamic_pointer_cast<IAxiomEvaluator>(std::make_shared<LiftedAxiomEvaluator>(parser.get_problem(), parser.get_pddl_repositories()));
    auto state_repository = StateRepository(axiom_evaluator);
    auto initial_state = state_repository.get_or_create_initial_state();
    auto initial_state_metric_value = compute_initial_state_metric_value(parser.get_problem());

    for (const auto& action : applicable_action_generator.create_applicable_action_generator(initial_state))
    {
        [[maybe_unused]] const auto [successor_state, successor_state_metric_value] =
            state_repository.get_or_create_successor_state(initial_state, action, initial_state_metric_value);

        // Attention: cannot nest more calls to create_applicable_action_generator because it is already in use.
    }
}

}
