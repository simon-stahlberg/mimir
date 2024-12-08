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

#include "mimir/search/algorithms/astar.hpp"

#include "mimir/formalism/parser.hpp"
#include "mimir/search/algorithms/astar/event_handlers.hpp"
#include "mimir/search/applicable_action_generators.hpp"
#include "mimir/search/applicable_action_generators/grounded/event_handlers.hpp"
#include "mimir/search/applicable_action_generators/lifted/event_handlers.hpp"
#include "mimir/search/axiom_evaluators.hpp"
#include "mimir/search/axiom_evaluators/grounded/event_handlers.hpp"
#include "mimir/search/axiom_evaluators/lifted/event_handlers.hpp"
#include "mimir/search/delete_relaxed_problem_explorator.hpp"
#include "mimir/search/heuristics.hpp"
#include "mimir/search/plan.hpp"
#include "mimir/search/state_repository.hpp"

#include <gtest/gtest.h>

namespace mimir::tests
{

enum class HeuristicType
{
    BLIND = 0,
    HSTAR = 1
};

/// @brief Instantiate a lifted AStar
class LiftedAStarPlanner
{
private:
    PDDLParser m_parser;

    std::shared_ptr<ILiftedApplicableActionGeneratorEventHandler> m_applicable_action_generator_event_handler;
    std::shared_ptr<LiftedApplicableActionGenerator> m_applicable_action_generator;
    std::shared_ptr<ILiftedAxiomEvaluatorEventHandler> m_axiom_evaluator_event_handler;
    std::shared_ptr<LiftedAxiomEvaluator> m_axiom_evaluator;
    std::shared_ptr<StateRepository> m_state_repository;
    std::shared_ptr<IHeuristic> m_heuristic;
    std::shared_ptr<IAStarAlgorithmEventHandler> m_astar_event_handler;
    std::unique_ptr<IAlgorithm> m_algorithm;

public:
    LiftedAStarPlanner(const fs::path& domain_file, const fs::path& problem_file, HeuristicType type) :
        m_parser(PDDLParser(domain_file, problem_file)),
        m_applicable_action_generator_event_handler(std::make_shared<DefaultLiftedApplicableActionGeneratorEventHandler>(false)),
        m_applicable_action_generator(std::make_shared<LiftedApplicableActionGenerator>(m_parser.get_problem(),
                                                                                        m_parser.get_pddl_repositories(),
                                                                                        m_applicable_action_generator_event_handler)),
        m_axiom_evaluator_event_handler(std::make_shared<DefaultLiftedAxiomEvaluatorEventHandler>()),
        m_axiom_evaluator(std::make_shared<LiftedAxiomEvaluator>(m_parser.get_problem(), m_parser.get_pddl_repositories(), m_axiom_evaluator_event_handler)),
        m_state_repository(std::make_shared<StateRepository>(m_axiom_evaluator)),
        m_astar_event_handler(std::make_shared<DefaultAStarAlgorithmEventHandler>(false))
    {
        switch (type)
        {
            case HeuristicType::BLIND:
            {
                m_heuristic = std::make_shared<BlindHeuristic>();
                break;
            }
            case HeuristicType::HSTAR:
            {
                m_heuristic = std::make_shared<HStarHeuristic>(m_parser.get_problem(),
                                                               m_parser.get_pddl_repositories(),
                                                               m_applicable_action_generator,
                                                               m_state_repository);
                break;
            }
            default:
            {
                throw std::runtime_error("Missing implementation for heuristic type");
            }
        }
        m_algorithm = std::make_unique<AStarAlgorithm>(m_applicable_action_generator, m_state_repository, m_heuristic, m_astar_event_handler);
    }

    SearchResult find_solution() { return m_algorithm->find_solution(); }

    const AStarAlgorithmStatistics& get_algorithm_statistics() const { return m_astar_event_handler->get_statistics(); }

    const LiftedApplicableActionGeneratorStatistics& get_applicable_action_generator_statistics() const
    {
        return m_applicable_action_generator_event_handler->get_statistics();
    }

    const LiftedAxiomEvaluatorStatistics& get_axiom_evaluator_statistics() const { return m_axiom_evaluator_event_handler->get_statistics(); }
};

/// @brief Instantiate a grounded AStar
class GroundedAStarPlanner
{
private:
    PDDLParser m_parser;

    DeleteRelaxedProblemExplorator m_delete_relaxed_problem_explorator;
    std::shared_ptr<IGroundedApplicableActionGeneratorEventHandler> m_applicable_action_generator_event_handler;
    std::shared_ptr<GroundedApplicableActionGenerator> m_applicable_action_generator;
    std::shared_ptr<IGroundedAxiomEvaluatorEventHandler> m_axiom_evaluator_event_handler;
    std::shared_ptr<GroundedAxiomEvaluator> m_axiom_evaluator;
    std::shared_ptr<StateRepository> m_state_repository;
    std::shared_ptr<IHeuristic> m_heuristic;
    std::shared_ptr<IAStarAlgorithmEventHandler> m_astar_event_handler;
    std::unique_ptr<IAlgorithm> m_algorithm;

public:
    GroundedAStarPlanner(const fs::path& domain_file, const fs::path& problem_file, HeuristicType type) :
        m_parser(PDDLParser(domain_file, problem_file)),
        m_delete_relaxed_problem_explorator(m_parser.get_problem(), m_parser.get_pddl_repositories()),
        m_applicable_action_generator_event_handler(std::make_shared<DefaultGroundedApplicableActionGeneratorEventHandler>()),
        m_applicable_action_generator(
            m_delete_relaxed_problem_explorator.create_grounded_applicable_action_generator(m_applicable_action_generator_event_handler)),
        m_axiom_evaluator_event_handler(std::make_shared<DefaultGroundedAxiomEvaluatorEventHandler>()),
        m_axiom_evaluator(m_delete_relaxed_problem_explorator.create_grounded_axiom_evaluator(m_axiom_evaluator_event_handler)),
        m_state_repository(std::make_shared<StateRepository>(m_axiom_evaluator)),
        m_astar_event_handler(std::make_shared<DefaultAStarAlgorithmEventHandler>())
    {
        switch (type)
        {
            case HeuristicType::BLIND:
            {
                m_heuristic = std::make_shared<BlindHeuristic>();
                break;
            }
            case HeuristicType::HSTAR:
            {
                m_heuristic = std::make_shared<HStarHeuristic>(m_parser.get_problem(),
                                                               m_parser.get_pddl_repositories(),
                                                               m_applicable_action_generator,
                                                               m_state_repository);
                break;
            }
            default:
            {
                throw std::runtime_error("Missing implementation for heuristic type");
            }
        }
        m_algorithm = std::make_unique<AStarAlgorithm>(m_applicable_action_generator, m_state_repository, m_heuristic, m_astar_event_handler);
    }

    SearchResult find_solution() { return m_algorithm->find_solution(); }

    const AStarAlgorithmStatistics& get_algorithm_statistics() const { return m_astar_event_handler->get_statistics(); }

    const GroundedApplicableActionGeneratorStatistics& get_applicable_action_generator_statistics() const
    {
        return m_applicable_action_generator_event_handler->get_statistics();
    }

    const GroundedAxiomEvaluatorStatistics& get_axiom_evaluator_statistics() const { return m_axiom_evaluator_event_handler->get_statistics(); }
};

/**
 * Gripper
 */

TEST(MimirTests, SearchAlgorithmsAStarGroundedHStarGripperTest)
{
    auto astar = GroundedAStarPlanner(fs::path(std::string(DATA_DIR) + "gripper/domain.pddl"),
                                      fs::path(std::string(DATA_DIR) + "gripper/test_problem.pddl"),
                                      HeuristicType::HSTAR);
    auto result = astar.find_solution();

    EXPECT_EQ(result.status, SearchStatus::SOLVED);
    EXPECT_EQ(result.plan.value().get_actions().size(), 3);

    const auto& astar_statistics = astar.get_algorithm_statistics();

    // Zero because AStar never enters a new f-layer.
    EXPECT_EQ(astar_statistics.get_num_generated_until_f_value().back(), 0);
    EXPECT_EQ(astar_statistics.get_num_expanded_until_f_value().back(), 0);
}

TEST(MimirTests, SearchAlgorithmsAStarLiftedHStarGripperTest)
{
    auto astar = LiftedAStarPlanner(fs::path(std::string(DATA_DIR) + "gripper/domain.pddl"),
                                    fs::path(std::string(DATA_DIR) + "gripper/test_problem.pddl"),
                                    HeuristicType::HSTAR);
    auto result = astar.find_solution();

    EXPECT_EQ(result.status, SearchStatus::SOLVED);
    EXPECT_EQ(result.plan.value().get_actions().size(), 3);

    const auto& astar_statistics = astar.get_algorithm_statistics();

    // Zero because AStar never enters a new f-layer.
    EXPECT_EQ(astar_statistics.get_num_generated_until_f_value().back(), 0);
    EXPECT_EQ(astar_statistics.get_num_expanded_until_f_value().back(), 0);
}

TEST(MimirTests, SearchAlgorithmsAStarGroundedBlindGripperTest)
{
    auto astar = GroundedAStarPlanner(fs::path(std::string(DATA_DIR) + "gripper/domain.pddl"),
                                      fs::path(std::string(DATA_DIR) + "gripper/test_problem.pddl"),
                                      HeuristicType::BLIND);
    auto result = astar.find_solution();

    EXPECT_EQ(result.status, SearchStatus::SOLVED);
    EXPECT_EQ(result.plan.value().get_actions().size(), 3);

    const auto& astar_statistics = astar.get_algorithm_statistics();

    // Identical to the BrFs result because domain has unit cost 1.
    EXPECT_EQ(astar_statistics.get_num_generated_until_f_value().back(), 44);
    EXPECT_EQ(astar_statistics.get_num_expanded_until_f_value().back(), 12);
}

TEST(MimirTests, SearchAlgorithmsAStarLiftedBlindGripperTest)
{
    auto astar = LiftedAStarPlanner(fs::path(std::string(DATA_DIR) + "gripper/domain.pddl"),
                                    fs::path(std::string(DATA_DIR) + "gripper/test_problem.pddl"),
                                    HeuristicType::BLIND);
    auto result = astar.find_solution();

    EXPECT_EQ(result.status, SearchStatus::SOLVED);
    EXPECT_EQ(result.plan.value().get_actions().size(), 3);

    const auto& astar_statistics = astar.get_algorithm_statistics();

    // Identical to the BrFs result because domain has unit cost 1.
    EXPECT_EQ(astar_statistics.get_num_generated_until_f_value().back(), 44);
    EXPECT_EQ(astar_statistics.get_num_expanded_until_f_value().back(), 12);
}

}
