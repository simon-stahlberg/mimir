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

#include "mimir/search/algorithms/siw.hpp"

#include "mimir/formalism/declarations.hpp"
#include "mimir/formalism/parser.hpp"
#include "mimir/search/algorithms/brfs/event_handlers.hpp"
#include "mimir/search/algorithms/iw/event_handlers.hpp"
#include "mimir/search/algorithms/siw/event_handlers.hpp"
#include "mimir/search/applicable_action_generators.hpp"
#include "mimir/search/applicable_action_generators/grounded/event_handlers.hpp"
#include "mimir/search/applicable_action_generators/lifted/event_handlers.hpp"
#include "mimir/search/axiom_evaluators.hpp"
#include "mimir/search/axiom_evaluators/grounded/event_handlers.hpp"
#include "mimir/search/axiom_evaluators/lifted/event_handlers.hpp"
#include "mimir/search/delete_relaxed_problem_explorator.hpp"
#include "mimir/search/plan.hpp"
#include "mimir/search/state_repository.hpp"

#include <gtest/gtest.h>

namespace mimir::tests
{

/// @brief Instantiate a lifted SIW search
class LiftedSIWPlanner
{
private:
    PDDLParser m_parser;

    std::shared_ptr<ILiftedApplicableActionGeneratorEventHandler> m_applicable_action_generator_event_handler;
    std::shared_ptr<LiftedApplicableActionGenerator> m_applicable_action_generator;
    std::shared_ptr<ILiftedAxiomEvaluatorEventHandler> m_axiom_evaluator_event_handler;
    std::shared_ptr<LiftedAxiomEvaluator> m_axiom_evaluator;
    std::shared_ptr<StateRepository> m_state_repository;
    std::shared_ptr<IBrFSAlgorithmEventHandler> m_brfs_event_handler;
    std::shared_ptr<IIWAlgorithmEventHandler> m_iw_event_handler;
    std::shared_ptr<ISIWAlgorithmEventHandler> m_siw_event_handler;
    std::unique_ptr<IAlgorithm> m_algorithm;

public:
    LiftedSIWPlanner(const fs::path& domain_file, const fs::path& problem_file, int arity) :
        m_parser(PDDLParser(domain_file, problem_file)),
        m_applicable_action_generator_event_handler(std::make_shared<DefaultLiftedApplicableActionGeneratorEventHandler>()),
        m_applicable_action_generator(std::make_shared<LiftedApplicableActionGenerator>(m_parser.get_problem(),
                                                                                        m_parser.get_pddl_repositories(),
                                                                                        m_applicable_action_generator_event_handler)),
        m_axiom_evaluator_event_handler(std::make_shared<DefaultLiftedAxiomEvaluatorEventHandler>()),
        m_axiom_evaluator(std::make_shared<LiftedAxiomEvaluator>(m_parser.get_problem(), m_parser.get_pddl_repositories(), m_axiom_evaluator_event_handler)),
        m_state_repository(std::make_shared<StateRepository>(m_axiom_evaluator)),
        m_brfs_event_handler(std::make_shared<DefaultBrFSAlgorithmEventHandler>()),
        m_iw_event_handler(std::make_shared<DefaultIWAlgorithmEventHandler>()),
        m_siw_event_handler(std::make_shared<DefaultSIWAlgorithmEventHandler>()),
        m_algorithm(std::make_unique<SIWAlgorithm>(m_applicable_action_generator,
                                                   m_state_repository,
                                                   arity,
                                                   m_brfs_event_handler,
                                                   m_iw_event_handler,
                                                   m_siw_event_handler))
    {
    }

    SearchResult find_solution() { return m_algorithm->find_solution(); }

    const SIWAlgorithmStatistics& get_iw_statistics() const { return m_siw_event_handler->get_statistics(); }

    const LiftedApplicableActionGeneratorStatistics& get_applicable_action_generator_statistics() const
    {
        return m_applicable_action_generator_event_handler->get_statistics();
    }

    const LiftedAxiomEvaluatorStatistics& get_axiom_evaluator_statistics() const { return m_axiom_evaluator_event_handler->get_statistics(); }
};

/// @brief Instantiate a grounded SIW search
class GroundedSIWPlanner
{
private:
    PDDLParser m_parser;

    DeleteRelaxedProblemExplorator m_delete_relaxed_problem_explorator;
    std::shared_ptr<IGroundedApplicableActionGeneratorEventHandler> m_applicable_action_generator_event_handler;
    std::shared_ptr<GroundedApplicableActionGenerator> m_applicable_action_generator;
    std::shared_ptr<IGroundedAxiomEvaluatorEventHandler> m_axiom_evaluator_event_handler;
    std::shared_ptr<GroundedAxiomEvaluator> m_axiom_evaluator;
    std::shared_ptr<StateRepository> m_state_repository;
    std::shared_ptr<IBrFSAlgorithmEventHandler> m_brfs_event_handler;
    std::shared_ptr<IIWAlgorithmEventHandler> m_iw_event_handler;
    std::shared_ptr<ISIWAlgorithmEventHandler> m_siw_event_handler;
    std::unique_ptr<IAlgorithm> m_algorithm;

public:
    GroundedSIWPlanner(const fs::path& domain_file, const fs::path& problem_file, int arity) :
        m_parser(PDDLParser(domain_file, problem_file)),
        m_delete_relaxed_problem_explorator(m_parser.get_problem(), m_parser.get_pddl_repositories()),
        m_applicable_action_generator_event_handler(std::make_shared<DefaultGroundedApplicableActionGeneratorEventHandler>()),
        m_applicable_action_generator(
            m_delete_relaxed_problem_explorator.create_grounded_applicable_action_generator(m_applicable_action_generator_event_handler)),
        m_axiom_evaluator_event_handler(std::make_shared<DefaultGroundedAxiomEvaluatorEventHandler>()),
        m_axiom_evaluator(m_delete_relaxed_problem_explorator.create_grounded_axiom_evaluator(m_axiom_evaluator_event_handler)),
        m_state_repository(std::make_shared<StateRepository>(m_axiom_evaluator)),
        m_brfs_event_handler(std::make_shared<DefaultBrFSAlgorithmEventHandler>()),
        m_iw_event_handler(std::make_shared<DefaultIWAlgorithmEventHandler>()),
        m_siw_event_handler(std::make_shared<DefaultSIWAlgorithmEventHandler>()),
        m_algorithm(std::make_unique<SIWAlgorithm>(m_applicable_action_generator,
                                                   m_state_repository,
                                                   arity,
                                                   m_brfs_event_handler,
                                                   m_iw_event_handler,
                                                   m_siw_event_handler))
    {
    }

    SearchResult find_solution() { return m_algorithm->find_solution(); }

    const SIWAlgorithmStatistics& get_iw_statistics() const { return m_siw_event_handler->get_statistics(); }

    const GroundedApplicableActionGeneratorStatistics& get_applicable_action_generator_statistics() const
    {
        return m_applicable_action_generator_event_handler->get_statistics();
    }

    const GroundedAxiomEvaluatorStatistics& get_axiom_evaluator_statistics() const { return m_axiom_evaluator_event_handler->get_statistics(); }
};

/**
 * Gripper
 */

TEST(MimirTests, SearchAlgorithmsSIWGroundedGripperTest)
{
    auto siw = GroundedSIWPlanner(fs::path(std::string(DATA_DIR) + "gripper/domain.pddl"), fs::path(std::string(DATA_DIR) + "gripper/test_problem2.pddl"), 3);
    const auto result = siw.find_solution();
    EXPECT_EQ(result.status, SearchStatus::SOLVED);
    EXPECT_EQ(result.plan.value().get_actions().size(), 7);

    const auto& applicable_action_generator_statistics = siw.get_applicable_action_generator_statistics();
    const auto& axiom_evaluator_statistics = siw.get_axiom_evaluator_statistics();

    EXPECT_EQ(applicable_action_generator_statistics.get_num_delete_free_reachable_fluent_ground_atoms(), 12);
    EXPECT_EQ(applicable_action_generator_statistics.get_num_delete_free_reachable_derived_ground_atoms(), 0);
    EXPECT_EQ(applicable_action_generator_statistics.get_num_delete_free_actions(), 20);
    EXPECT_EQ(axiom_evaluator_statistics.get_num_delete_free_axioms(), 0);

    EXPECT_EQ(applicable_action_generator_statistics.get_num_ground_actions(), 20);
    EXPECT_EQ(applicable_action_generator_statistics.get_num_nodes_in_action_match_tree(), 48);

    EXPECT_EQ(axiom_evaluator_statistics.get_num_ground_axioms(), 0);
    EXPECT_EQ(axiom_evaluator_statistics.get_num_nodes_in_axiom_match_tree(), 1);

    const auto& siw_statistics = siw.get_iw_statistics();

    EXPECT_EQ(siw_statistics.get_iw_statistics_by_subproblem().size(), 2);
    EXPECT_EQ(siw_statistics.get_num_generated_until_last_g_layer(), 94);
    EXPECT_EQ(siw_statistics.get_num_expanded_until_last_g_layer(), 25);
    EXPECT_EQ(siw_statistics.get_maximum_effective_width(), 2);
    EXPECT_EQ(siw_statistics.get_average_effective_width(), 2);
}

TEST(MimirTests, SearchAlgorithmsSIWLiftedGripperTest)
{
    auto siw = LiftedSIWPlanner(fs::path(std::string(DATA_DIR) + "gripper/domain.pddl"), fs::path(std::string(DATA_DIR) + "gripper/test_problem2.pddl"), 3);
    const auto result = siw.find_solution();
    EXPECT_EQ(result.status, SearchStatus::SOLVED);
    EXPECT_EQ(result.plan.value().get_actions().size(), 7);

    const auto& applicable_action_generator_statistics = siw.get_applicable_action_generator_statistics();
    const auto& axiom_evaluator_statistics = siw.get_axiom_evaluator_statistics();

    EXPECT_EQ(applicable_action_generator_statistics.get_num_ground_action_cache_hits_per_search_layer().back(), 158);
    EXPECT_EQ(applicable_action_generator_statistics.get_num_ground_action_cache_misses_per_search_layer().back(), 18);

    EXPECT_EQ(axiom_evaluator_statistics.get_num_ground_axiom_cache_hits_per_search_layer().back(), 0);
    EXPECT_EQ(axiom_evaluator_statistics.get_num_ground_axiom_cache_misses_per_search_layer().back(), 0);

    const auto& siw_statistics = siw.get_iw_statistics();

    EXPECT_EQ(siw_statistics.get_iw_statistics_by_subproblem().size(), 2);
    EXPECT_EQ(siw_statistics.get_num_generated_until_last_g_layer(), 94);
    EXPECT_EQ(siw_statistics.get_num_expanded_until_last_g_layer(), 25);
    EXPECT_EQ(siw_statistics.get_maximum_effective_width(), 2);
    EXPECT_EQ(siw_statistics.get_average_effective_width(), 2);
}

}
