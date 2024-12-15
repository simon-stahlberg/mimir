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
#include "mimir/search/applicable_action_generators.hpp"
#include "mimir/search/axiom_evaluators.hpp"
#include "mimir/search/grounders/grounder.hpp"

#include <gtest/gtest.h>

namespace mimir::tests
{

TEST(MimirTests, SearchStateRepositoryTest)
{
    // Instantiate lifted version
    const auto domain_file = fs::path(std::string(DATA_DIR) + "gripper/domain.pddl");
    const auto problem_file = fs::path(std::string(DATA_DIR) + "gripper/test_problem.pddl");
    PDDLParser parser(domain_file, problem_file);
    const auto grounder = std::make_shared<Grounder>(parser.get_problem(), parser.get_pddl_repositories());
    auto applicable_action_generator = LiftedApplicableActionGenerator(grounder->get_action_grounder());
    const auto axiom_evaluator = std::dynamic_pointer_cast<IAxiomEvaluator>(std::make_shared<LiftedAxiomEvaluator>(grounder->get_axiom_grounder()));
    auto state_repository = StateRepository(axiom_evaluator);
    auto state_repository_workspace = StateRepositoryWorkspace();
    auto initial_state = state_repository.get_or_create_initial_state(state_repository_workspace);

    auto applicable_action_generator_workspace = ApplicableActionGeneratorWorkspace();
    for (const auto& action : applicable_action_generator.create_applicable_action_generator(initial_state, applicable_action_generator_workspace))
    {
        const auto [successor_state, action_cost] = state_repository.get_or_create_successor_state(initial_state, action, state_repository_workspace);

        // Attention!: we need to initialize another ApplicableActionGeneratorWorkspace because the outter one hasnt finished processing yet.
        auto applicable_action_generator_workspace2 = ApplicableActionGeneratorWorkspace();
        for (const auto& action2 : applicable_action_generator.create_applicable_action_generator(successor_state, applicable_action_generator_workspace2))
        {
            [[maybe_unused]] const auto [successor_state2, action_cost2] =
                state_repository.get_or_create_successor_state(successor_state, action2, state_repository_workspace);
        }
    }
}

}
