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

#include "mimir/formalism/parser.hpp"
#include "mimir/search/algorithms.hpp"
#include "mimir/search/applicable_action_generators.hpp"
#include "mimir/search/axiom_evaluators.hpp"
#include "mimir/search/delete_relaxed_problem_explorator.hpp"
#include "mimir/search/plan.hpp"
#include "mimir/search/state_repository.hpp"

#include <gtest/gtest.h>

namespace mimir::tests
{

TEST(MimirTests, SearchApplicableActionGeneratorsGroundedTest)
{
    const auto domain_file = fs::path(std::string(DATA_DIR) + "miconic-fulladl/domain.pddl");
    const auto problem_file = fs::path(std::string(DATA_DIR) + "miconic-fulladl/test_problem.pddl");
    PDDLParser parser(domain_file, problem_file);
    auto delete_free_problem_explorator = DeleteRelaxedProblemExplorator(parser.get_problem(), parser.get_pddl_repositories());
    auto applicable_action_generator_event_handler = std::make_shared<DefaultGroundedApplicableActionGeneratorEventHandler>();
    auto applicable_action_generator = delete_free_problem_explorator.create_grounded_applicable_action_generator(applicable_action_generator_event_handler);
    auto axiom_evaluator_event_handler = std::make_shared<DefaultGroundedAxiomEvaluatorEventHandler>();
    auto axiom_evaluator =
        std::dynamic_pointer_cast<IAxiomEvaluator>(delete_free_problem_explorator.create_grounded_axiom_evaluator(axiom_evaluator_event_handler));
    auto state_repository = std::make_shared<StateRepository>(axiom_evaluator);
    auto brfs_event_handler = std::make_shared<DefaultBrFSAlgorithmEventHandler>();
    auto brfs = BrFSAlgorithm(applicable_action_generator, state_repository, brfs_event_handler);
    const auto result = brfs.find_solution();
    EXPECT_EQ(result.status, SearchStatus::SOLVED);

    const auto& applicable_action_generator_statistics = applicable_action_generator_event_handler->get_statistics();
    const auto& axiom_evaluator_statistics = axiom_evaluator_event_handler->get_statistics();
    EXPECT_EQ(applicable_action_generator_statistics.get_num_delete_free_reachable_fluent_ground_atoms(), 9);
    EXPECT_EQ(applicable_action_generator_statistics.get_num_delete_free_reachable_derived_ground_atoms(), 8);
    EXPECT_EQ(applicable_action_generator_statistics.get_num_delete_free_actions(), 7);
    EXPECT_EQ(axiom_evaluator_statistics.get_num_delete_free_axioms(), 20);

    EXPECT_EQ(applicable_action_generator_statistics.get_num_ground_actions(), 10);
    EXPECT_EQ(applicable_action_generator_statistics.get_num_nodes_in_action_match_tree(), 45);

    EXPECT_EQ(axiom_evaluator_statistics.get_num_ground_axioms(), 16);
    EXPECT_EQ(axiom_evaluator_statistics.get_num_nodes_in_axiom_match_tree(), 12);

    const auto& brfs_statistics = brfs_event_handler->get_statistics();

    EXPECT_EQ(brfs_statistics.get_num_generated_until_g_value().back(), 105);
    EXPECT_EQ(brfs_statistics.get_num_expanded_until_g_value().back(), 41);
}

}
