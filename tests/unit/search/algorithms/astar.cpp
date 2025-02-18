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

#include "mimir/formalism/repositories.hpp"
#include "mimir/search/algorithms.hpp"
#include "mimir/search/applicable_action_generators.hpp"
#include "mimir/search/axiom_evaluators.hpp"
#include "mimir/search/delete_relaxed_problem_explorator.hpp"
#include "mimir/search/heuristics.hpp"
#include "mimir/search/plan.hpp"
#include "mimir/search/search_context.hpp"
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
    ProblemContext m_problem_context;
    std::shared_ptr<ILiftedApplicableActionGeneratorEventHandler> m_applicable_action_generator_event_handler;
    std::shared_ptr<LiftedApplicableActionGenerator> m_applicable_action_generator;
    std::shared_ptr<ILiftedAxiomEvaluatorEventHandler> m_axiom_evaluator_event_handler;
    std::shared_ptr<LiftedAxiomEvaluator> m_axiom_evaluator;
    std::shared_ptr<StateRepository> m_state_repository;
    std::shared_ptr<IHeuristic> m_heuristic;
    std::shared_ptr<IAStarAlgorithmEventHandler> m_astar_event_handler;
    SearchContext m_search_context;

public:
    LiftedAStarPlanner(const fs::path& domain_file, const fs::path& problem_file, HeuristicType type) :
        m_problem_context(domain_file, problem_file),
        m_applicable_action_generator_event_handler(std::make_shared<DefaultLiftedApplicableActionGeneratorEventHandler>()),
        m_applicable_action_generator(std::make_shared<LiftedApplicableActionGenerator>(m_problem_context, m_applicable_action_generator_event_handler)),
        m_axiom_evaluator_event_handler(std::make_shared<DefaultLiftedAxiomEvaluatorEventHandler>()),
        m_axiom_evaluator(std::make_shared<LiftedAxiomEvaluator>(m_problem_context, m_axiom_evaluator_event_handler)),
        m_state_repository(std::make_shared<StateRepository>(m_axiom_evaluator)),
        m_astar_event_handler(std::make_shared<DefaultAStarAlgorithmEventHandler>(false)),
        m_search_context(m_problem_context, m_applicable_action_generator, m_state_repository)
    {
        switch (type)
        {
            case HeuristicType::BLIND:
            {
                m_heuristic = std::make_shared<BlindHeuristic>(m_problem_context.get_problem());
                break;
            }
            case HeuristicType::HSTAR:
            {
                m_heuristic = std::make_shared<HStarHeuristic>(m_search_context);
                break;
            }
            default:
            {
                throw std::runtime_error("Missing implementation for heuristic type");
            }
        }
    }

    SearchResult find_solution() { return find_solution_astar(m_search_context, m_heuristic, std::nullopt, m_astar_event_handler); }

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
    ProblemContext m_problem_context;
    DeleteRelaxedProblemExplorator m_delete_relaxed_problem_explorator;
    std::shared_ptr<IGroundedApplicableActionGeneratorEventHandler> m_applicable_action_generator_event_handler;
    std::shared_ptr<GroundedApplicableActionGenerator> m_applicable_action_generator;
    std::shared_ptr<IGroundedAxiomEvaluatorEventHandler> m_axiom_evaluator_event_handler;
    std::shared_ptr<GroundedAxiomEvaluator> m_axiom_evaluator;
    std::shared_ptr<StateRepository> m_state_repository;
    std::shared_ptr<IHeuristic> m_heuristic;
    std::shared_ptr<IAStarAlgorithmEventHandler> m_astar_event_handler;
    SearchContext m_search_context;

public:
    GroundedAStarPlanner(const fs::path& domain_file, const fs::path& problem_file, HeuristicType type) :
        m_problem_context(domain_file, problem_file),
        m_delete_relaxed_problem_explorator(m_problem_context),
        m_applicable_action_generator_event_handler(std::make_shared<DefaultGroundedApplicableActionGeneratorEventHandler>()),
        m_applicable_action_generator(
            m_delete_relaxed_problem_explorator.create_grounded_applicable_action_generator(match_tree::Options(),
                                                                                            m_applicable_action_generator_event_handler)),
        m_axiom_evaluator_event_handler(std::make_shared<DefaultGroundedAxiomEvaluatorEventHandler>()),
        m_axiom_evaluator(m_delete_relaxed_problem_explorator.create_grounded_axiom_evaluator(match_tree::Options(), m_axiom_evaluator_event_handler)),
        m_state_repository(std::make_shared<StateRepository>(m_axiom_evaluator)),
        m_astar_event_handler(std::make_shared<DefaultAStarAlgorithmEventHandler>()),
        m_search_context(m_problem_context, m_applicable_action_generator, m_state_repository)
    {
        switch (type)
        {
            case HeuristicType::BLIND:
            {
                m_heuristic = std::make_shared<BlindHeuristic>(m_problem_context.get_problem());
                break;
            }
            case HeuristicType::HSTAR:
            {
                m_heuristic = std::make_shared<HStarHeuristic>(m_search_context);
                break;
            }
            default:
            {
                throw std::runtime_error("Missing implementation for heuristic type");
            }
        }
    }

    SearchResult find_solution() { return find_solution_astar(m_search_context, m_heuristic, std::nullopt, m_astar_event_handler); }

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
    EXPECT_EQ(astar_statistics.get_num_generated_until_f_value().rbegin()->second, 0);
    EXPECT_EQ(astar_statistics.get_num_expanded_until_f_value().rbegin()->second, 0);
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
    EXPECT_EQ(astar_statistics.get_num_generated_until_f_value().rbegin()->second, 0);
    EXPECT_EQ(astar_statistics.get_num_expanded_until_f_value().rbegin()->second, 0);
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
    EXPECT_EQ(astar_statistics.get_num_generated_until_f_value().rbegin()->second, 24);
    EXPECT_EQ(astar_statistics.get_num_expanded_until_f_value().rbegin()->second, 6);
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
    EXPECT_EQ(astar_statistics.get_num_generated_until_f_value().rbegin()->second, 24);
    EXPECT_EQ(astar_statistics.get_num_expanded_until_f_value().rbegin()->second, 6);
}

}
