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

#include "mimir/formalism/problem.hpp"
#include "mimir/formalism/repositories.hpp"
#include "mimir/search/algorithms.hpp"
#include "mimir/search/applicable_action_generators.hpp"
#include "mimir/search/axiom_evaluators.hpp"
#include "mimir/search/delete_relaxed_problem_explorator.hpp"
#include "mimir/search/plan.hpp"
#include "mimir/search/search_context.hpp"
#include "mimir/search/state_repository.hpp"

#include <gtest/gtest.h>

using namespace mimir::search;
using namespace mimir::formalism;

namespace mimir::tests
{

TEST(MimirTests, SearchApplicableActionGeneratorsGroundedTest)
{
    const auto domain_file = fs::path(std::string(DATA_DIR) + "miconic-fulladl/domain.pddl");
    const auto problem_file = fs::path(std::string(DATA_DIR) + "miconic-fulladl/test_problem.pddl");
    const auto problem = ProblemImpl::create(domain_file, problem_file);

    auto delete_free_problem_explorator = DeleteRelaxedProblemExplorator(problem);
    const auto applicable_action_generator_event_handler = GroundedApplicableActionGeneratorImpl::DefaultEventHandlerImpl::create();
    const auto applicable_action_generator =
        delete_free_problem_explorator.create_grounded_applicable_action_generator(match_tree::Options(), applicable_action_generator_event_handler);
    const auto axiom_evaluator_event_handler = GroundedAxiomEvaluatorImpl::DefaultEventHandlerImpl::create();
    const auto axiom_evaluator = delete_free_problem_explorator.create_grounded_axiom_evaluator(match_tree::Options(), axiom_evaluator_event_handler);
    const auto state_repository = StateRepositoryImpl::create(axiom_evaluator);
    const auto brfs_event_handler = brfs::DefaultEventHandlerImpl::create(problem);
    const auto search_context = SearchContextImpl::create(problem, applicable_action_generator, state_repository);

    const auto result = brfs::find_solution(search_context, nullptr, brfs_event_handler);
    EXPECT_EQ(result.status, SearchStatus::SOLVED);

    const auto& applicable_action_generator_statistics = applicable_action_generator_event_handler->get_statistics();
    EXPECT_EQ(applicable_action_generator_statistics.statistics.num_nodes, 34);

    const auto& axiom_evaluator_statistics = axiom_evaluator_event_handler->get_statistics();
    EXPECT_EQ(axiom_evaluator_statistics.m_match_tree_partition_statistics.front().num_nodes, 11);

    const auto& brfs_statistics = brfs_event_handler->get_statistics();
    EXPECT_EQ(brfs_statistics.get_num_generated_until_g_value().back(), 105);
    EXPECT_EQ(brfs_statistics.get_num_expanded_until_g_value().back(), 41);
}

}
