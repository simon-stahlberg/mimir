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
#include "mimir/search/grounders.hpp"
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

    auto delete_free_problem_explorator = LiftedGrounder(problem);
    const auto applicable_action_generator_event_handler = GroundedApplicableActionGeneratorImpl::DefaultEventHandlerImpl::create();
    const auto applicable_action_generator =
        delete_free_problem_explorator.create_grounded_applicable_action_generator(match_tree::Options(), applicable_action_generator_event_handler);
    const auto axiom_evaluator_event_handler = GroundedAxiomEvaluatorImpl::DefaultEventHandlerImpl::create();
    const auto axiom_evaluator = delete_free_problem_explorator.create_grounded_axiom_evaluator(match_tree::Options(), axiom_evaluator_event_handler);
    const auto state_repository = StateRepositoryImpl::create(axiom_evaluator);
    const auto brfs_event_handler = brfs::DefaultEventHandlerImpl::create(problem);
    const auto search_context = SearchContextImpl::create(problem, applicable_action_generator, state_repository);

    auto brfs_options = brfs::Options();
    brfs_options.event_handler = brfs_event_handler;

    const auto result = brfs::find_solution(search_context, brfs_options);
    EXPECT_EQ(result.status, SearchStatus::SOLVED);

    const auto [initial_state, initial_metric_value] = state_repository->get_or_create_initial_state();
    auto first_applicable_action = GroundAction { nullptr };
    for (const auto& action : applicable_action_generator->create_applicable_action_generator(initial_state))
    {
        first_applicable_action = action;
        break;
    }

    ASSERT_NE(first_applicable_action, nullptr);

    const auto& applicable_action_generator_statistics = applicable_action_generator_event_handler->get_statistics();
    EXPECT_EQ(applicable_action_generator_statistics.statistics.num_nodes, 34);

    const auto& axiom_evaluator_statistics = axiom_evaluator_event_handler->get_statistics();
    EXPECT_EQ(axiom_evaluator_statistics.m_match_tree_partition_statistics.front().num_nodes, 11);

    const auto& brfs_statistics = brfs_event_handler->get_statistics();
    EXPECT_EQ(brfs_statistics.get_num_generated_until_g_value().back(), 105);
    EXPECT_EQ(brfs_statistics.get_num_expanded_until_g_value().back(), 41);
    (void) initial_metric_value;  // Silence unused variable warning
}

TEST(MimirTests, SearchApplicableActionGeneratorsGroundedNoveltyTest)
{
    const auto domain_content = std::string(
        "(define (domain grounded-novelty-test) "
        "    (:requirements :strips) "
        "    (:predicates (p ?x) (q ?x)) "
        "    (:action make-q "
        "        :parameters (?x) "
        "        :precondition (and (p ?x)) "
        "        :effect (and (q ?x))) "
        ")");

    const auto problem_content = std::string(
        "(define (problem grounded-novelty-problem) "
        "    (:domain grounded-novelty-test) "
        "    (:objects a) "
        "    (:init (p a)) "
        "    (:goal (q a)) "
        ")");

    const auto problem = ProblemImpl::create(domain_content, "", problem_content, "");
    const auto novelty_generator = GroundedNoveltyApplicableActionGeneratorImpl::create(problem);

    auto search_context = SearchContextImpl::create(problem, SearchContextImpl::Options(SearchContextImpl::GroundedOptions()));
    auto& original_state_repository = *search_context->get_state_repository();
    const auto [initial_state, initial_metric_value] = original_state_repository.get_or_create_initial_state();

    auto applicable_actions = GroundActionList {};
    for (const auto& action : novelty_generator->create_applicable_action_generator(initial_state))
    {
        applicable_actions.push_back(action);
    }

    ASSERT_EQ(applicable_actions.size(), 1);
    EXPECT_EQ(applicable_actions.front()->get_action()->get_name(), "make-q");

    const auto [successor_state, successor_metric_value] =
        original_state_repository.get_or_create_successor_state(initial_state, applicable_actions.front(), initial_metric_value);

    auto successor_applicable_actions = GroundActionList {};
    for (const auto& action : novelty_generator->create_applicable_action_generator(successor_state))
    {
        successor_applicable_actions.push_back(action);
    }

    novelty_generator->update_novelty_history(applicable_actions.front());

    auto blocked_actions = GroundActionList {};
    for (const auto& action : novelty_generator->create_applicable_action_generator(initial_state))
    {
        blocked_actions.push_back(action);
    }

    EXPECT_TRUE(blocked_actions.empty());

    novelty_generator->reset_novelty_history();

    auto reset_applicable_actions = GroundActionList {};
    for (const auto& action : novelty_generator->create_applicable_action_generator(initial_state))
    {
        reset_applicable_actions.push_back(action);
    }

    ASSERT_EQ(reset_applicable_actions.size(), 1);
    EXPECT_EQ(reset_applicable_actions.front()->get_action()->get_name(), "make-q");
    (void) successor_metric_value;
}

TEST(MimirTests, SearchApplicableActionGeneratorsGroundedNoveltyMinimalTest)
{
    // A domain where two actions produce the exact same novel effect
    const auto domain_content = std::string(
        "(define (domain grounded-novelty-minimal-test) "
        "    (:requirements :strips) "
        "    (:predicates (p ?x) (q ?x) (r ?x)) "
        "    (:action make-q-1 "
        "        :parameters (?x) "
        "        :precondition (and (p ?x)) "
        "        :effect (and (q ?x))) "
        "    (:action make-q-2 "
        "        :parameters (?x) "
        "        :precondition (and (r ?x)) "
        "        :effect (and (q ?x))) "
        ")");

    // Initial state satisfies preconditions for both actions
    const auto problem_content = std::string(
        "(define (problem grounded-novelty-minimal-problem) "
        "    (:domain grounded-novelty-minimal-test) "
        "    (:objects a) "
        "    (:init (p a) (r a)) "
        "    (:goal (q a)) "
        ")");

    const auto problem = ProblemImpl::create(domain_content, "", problem_content, "");

    // Test Exhaustive Mode: Should return both actions because they are both applicable
    // and both produce the novel effect (q a).
    {
        const auto exhaustive_generator = GroundedNoveltyApplicableActionGeneratorImpl::create(
            problem, 1, match_tree::Options(), nullptr, GroundedNoveltyApplicableActionGeneratorImpl::Mode::Exhaustive);

        auto search_context = SearchContextImpl::create(problem, SearchContextImpl::Options(SearchContextImpl::GroundedOptions()));
        auto& original_state_repository = *search_context->get_state_repository();
        const auto [initial_state, initial_metric_value] = original_state_repository.get_or_create_initial_state();

        auto applicable_actions = GroundActionList {};
        for (const auto& action : exhaustive_generator->create_applicable_action_generator(initial_state))
        {
            applicable_actions.push_back(action);
        }

        EXPECT_EQ(applicable_actions.size(), 2);
        (void) initial_metric_value;
    }

    // Test Minimal Mode: Should return only ONE action because the second one 
    // produces overlapping novel atoms (q a) which the first one already covered.
    {
        const auto minimal_generator = GroundedNoveltyApplicableActionGeneratorImpl::create(
            problem, 1, match_tree::Options(), nullptr, GroundedNoveltyApplicableActionGeneratorImpl::Mode::Minimal);

        auto search_context = SearchContextImpl::create(problem, SearchContextImpl::Options(SearchContextImpl::GroundedOptions()));
        auto& original_state_repository = *search_context->get_state_repository();
        const auto [initial_state, initial_metric_value] = original_state_repository.get_or_create_initial_state();

        auto applicable_actions = GroundActionList {};
        for (const auto& action : minimal_generator->create_applicable_action_generator(initial_state))
        {
            applicable_actions.push_back(action);
        }

        // Only one of make-q-1 or make-q-2 should be generated.
        EXPECT_EQ(applicable_actions.size(), 1);
        
        // Let's also test that we can update the history properly with the chosen action
        // and subsequent generations on the same state return empty
        minimal_generator->update_novelty_history(applicable_actions.front());
        
        auto subsequent_actions = GroundActionList {};
        for (const auto& action : minimal_generator->create_applicable_action_generator(initial_state))
        {
            subsequent_actions.push_back(action);
        }
        
        EXPECT_TRUE(subsequent_actions.empty());
        (void) initial_metric_value;
    }
}

TEST(MimirTests, SearchApplicableActionGeneratorsGroundedNoveltyExceptionsTest)
{
    // A domain to test exception throwing
    const auto domain_content = std::string(
        "(define (domain novelty-exceptions-test) "
        "    (:requirements :strips :conditional-effects) "
        "    (:predicates (p ?x) (q ?x) (z)) "
        "    (:action make-q "
        "        :parameters (?x) "
        "        :precondition (and (p ?x)) "
        "        :effect (and (q ?x))) "
        "    (:action cond-action "
        "        :parameters (?x) "
        "        :precondition () "
        "        :effect (and (when (p ?x) (z)))) "
        ")");

    const auto problem_content = std::string(
        "(define (problem novelty-exceptions-problem) "
        "    (:domain novelty-exceptions-test) "
        "    (:objects a) "
        "    (:init (p a)) "
        "    (:goal (q a)) "
        ")");

    const auto problem = ProblemImpl::create(domain_content, "", problem_content, "");

    // 1. Test Exception for Conditional Effects
    EXPECT_THROW(GroundedNoveltyApplicableActionGeneratorImpl::create(problem), std::runtime_error);

    // Create a problem without conditional effects for the rest of the tests
    const auto valid_domain_content = std::string(
        "(define (domain valid-test) "
        "    (:requirements :strips) "
        "    (:predicates (p ?x) (q ?x)) "
        "    (:action make-q "
        "        :parameters (?x) "
        "        :precondition (and (p ?x)) "
        "        :effect (and (q ?x))) "
        ")");

    const auto valid_problem_content = std::string(
        "(define (problem valid-problem) "
        "    (:domain valid-test) "
        "    (:objects a) "
        "    (:init (p a)) "
        "    (:goal (q a)) "
        ")");

    const auto valid_problem = ProblemImpl::create(valid_domain_content, "", valid_problem_content, "");
    auto novelty_generator = GroundedNoveltyApplicableActionGeneratorImpl::create(valid_problem);

    // 2. Test Invalid State Problem
    const auto other_problem = ProblemImpl::create(valid_domain_content, "", valid_problem_content, "");
    auto search_context = SearchContextImpl::create(other_problem, SearchContextImpl::Options(SearchContextImpl::GroundedOptions()));
    auto& other_state_repository = *search_context->get_state_repository();
    const auto [invalid_state, _] = other_state_repository.get_or_create_initial_state();

    EXPECT_THROW({
        for (const auto& action : novelty_generator->create_applicable_action_generator(invalid_state))
        {
            (void) action;
        }
    }, std::runtime_error);

    // 3. Test Unmapped Action Update
    auto other_grounder = LiftedGrounder(other_problem);
    const auto other_action = other_grounder.create_ground_actions().front();
    EXPECT_THROW(novelty_generator->update_novelty_history(other_action), std::runtime_error);
}

}
