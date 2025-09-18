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

#include "mimir/search/algorithms/astar_eager.hpp"

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

using namespace mimir::search;
using namespace mimir::formalism;

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
    Problem m_problem;
    KPKCLiftedApplicableActionGeneratorImpl::EventHandler m_applicable_action_generator_event_handler;
    KPKCLiftedApplicableActionGenerator m_applicable_action_generator;
    KPKCLiftedAxiomEvaluatorImpl::EventHandler m_axiom_evaluator_event_handler;
    KPKCLiftedAxiomEvaluator m_axiom_evaluator;
    StateRepository m_state_repository;
    Heuristic m_heuristic;
    astar_eager::EventHandler m_astar_event_handler;
    SearchContext m_search_context;

public:
    LiftedAStarPlanner(const fs::path& domain_file, const fs::path& problem_file, HeuristicType type) :
        m_problem(ProblemImpl::create(domain_file, problem_file)),
        m_applicable_action_generator_event_handler(KPKCLiftedApplicableActionGeneratorImpl::DefaultEventHandlerImpl::create()),
        m_applicable_action_generator(KPKCLiftedApplicableActionGeneratorImpl::create(m_problem, m_applicable_action_generator_event_handler)),
        m_axiom_evaluator_event_handler(KPKCLiftedAxiomEvaluatorImpl::DefaultEventHandlerImpl::create()),
        m_axiom_evaluator(KPKCLiftedAxiomEvaluatorImpl::create(m_problem, m_axiom_evaluator_event_handler)),
        m_state_repository(StateRepositoryImpl::create(m_axiom_evaluator)),
        m_astar_event_handler(astar_eager::DefaultEventHandlerImpl::create(m_problem)),
        m_search_context(SearchContextImpl::create(m_problem, m_applicable_action_generator, m_state_repository))
    {
        switch (type)
        {
            case HeuristicType::BLIND:
            {
                m_heuristic = BlindHeuristicImpl::create(m_problem);
                break;
            }
            case HeuristicType::HSTAR:
            {
                m_heuristic = PerfectHeuristicImpl::create(m_search_context);
                break;
            }
            default:
            {
                throw std::runtime_error("Missing implementation for heuristic type");
            }
        }
    }

    SearchResult find_solution()
    {
        auto astar_options = astar_eager::Options();
        astar_options.event_handler = m_astar_event_handler;

        return astar_eager::find_solution(m_search_context, m_heuristic, astar_options);
    }

    const astar_eager::Statistics& get_algorithm_statistics() const { return m_astar_event_handler->get_statistics(); }

    const KPKCLiftedApplicableActionGeneratorImpl::Statistics& get_applicable_action_generator_statistics() const
    {
        return m_applicable_action_generator_event_handler->get_statistics();
    }

    const KPKCLiftedAxiomEvaluatorImpl::Statistics& get_axiom_evaluator_statistics() const { return m_axiom_evaluator_event_handler->get_statistics(); }
};

/// @brief Instantiate a grounded AStar
class GroundedAStarPlanner
{
private:
    Problem m_problem;
    DeleteRelaxedProblemExplorator m_delete_relaxed_problem_explorator;
    GroundedApplicableActionGeneratorImpl::EventHandler m_applicable_action_generator_event_handler;
    GroundedApplicableActionGenerator m_applicable_action_generator;
    GroundedAxiomEvaluatorImpl::EventHandler m_axiom_evaluator_event_handler;
    GroundedAxiomEvaluator m_axiom_evaluator;
    StateRepository m_state_repository;
    Heuristic m_heuristic;
    astar_eager::EventHandler m_astar_event_handler;
    SearchContext m_search_context;

public:
    GroundedAStarPlanner(const fs::path& domain_file, const fs::path& problem_file, HeuristicType type) :
        m_problem(ProblemImpl::create(domain_file, problem_file)),
        m_delete_relaxed_problem_explorator(m_problem),
        m_applicable_action_generator_event_handler(GroundedApplicableActionGeneratorImpl::DefaultEventHandlerImpl::create()),
        m_applicable_action_generator(
            m_delete_relaxed_problem_explorator.create_grounded_applicable_action_generator(match_tree::Options(),
                                                                                            m_applicable_action_generator_event_handler)),
        m_axiom_evaluator_event_handler(GroundedAxiomEvaluatorImpl::DefaultEventHandlerImpl::create()),
        m_axiom_evaluator(m_delete_relaxed_problem_explorator.create_grounded_axiom_evaluator(match_tree::Options(), m_axiom_evaluator_event_handler)),
        m_state_repository(StateRepositoryImpl::create(m_axiom_evaluator)),
        m_astar_event_handler(astar_eager::DefaultEventHandlerImpl::create(m_problem)),
        m_search_context(SearchContextImpl::create(m_problem, m_applicable_action_generator, m_state_repository))
    {
        switch (type)
        {
            case HeuristicType::BLIND:
            {
                m_heuristic = BlindHeuristicImpl::create(m_problem);
                break;
            }
            case HeuristicType::HSTAR:
            {
                m_heuristic = PerfectHeuristicImpl::create(m_search_context);
                break;
            }
            default:
            {
                throw std::runtime_error("Missing implementation for heuristic type");
            }
        }
    }

    SearchResult find_solution()
    {
        auto astar_options = astar_eager::Options();
        astar_options.event_handler = m_astar_event_handler;

        return astar_eager::find_solution(m_search_context, m_heuristic, astar_options);
    }

    const astar_eager::Statistics& get_algorithm_statistics() const { return m_astar_event_handler->get_statistics(); }

    const GroundedApplicableActionGeneratorImpl::Statistics& get_applicable_action_generator_statistics() const
    {
        return m_applicable_action_generator_event_handler->get_statistics();
    }

    const GroundedAxiomEvaluatorImpl::Statistics& get_axiom_evaluator_statistics() const { return m_axiom_evaluator_event_handler->get_statistics(); }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Classical planning
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Gripper
 */

TEST(MimirTests, SearchAlgorithmsAStarGroundedPerfectGripperTest)
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

TEST(MimirTests, SearchAlgorithmsAStarLiftedPerfectGripperTest)
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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Numeric planning
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Fo-counters
 */

TEST(MimirTests, SearchAlgorithmsAStarGroundedBlindFoCountersTest)
{
    auto astar = GroundedAStarPlanner(fs::path(std::string(DATA_DIR) + "fo-counters/domain.pddl"),
                                      fs::path(std::string(DATA_DIR) + "fo-counters/test_problem.pddl"),
                                      HeuristicType::BLIND);
    auto result = astar.find_solution();

    EXPECT_EQ(result.status, SearchStatus::SOLVED);
    EXPECT_EQ(result.plan.value().get_actions().size(), 5);
    EXPECT_EQ(result.plan.value().get_cost(), 5);

    const auto& astar_statistics = astar.get_algorithm_statistics();

    EXPECT_EQ(astar_statistics.get_num_generated_until_f_value().rbegin()->second, 1071);
    EXPECT_EQ(astar_statistics.get_num_expanded_until_f_value().rbegin()->second, 113);
}

TEST(MimirTests, SearchAlgorithmsAStarLiftedBlindFoCountersTest)
{
    auto astar = LiftedAStarPlanner(fs::path(std::string(DATA_DIR) + "fo-counters/domain.pddl"),
                                    fs::path(std::string(DATA_DIR) + "fo-counters/test_problem.pddl"),
                                    HeuristicType::BLIND);
    auto result = astar.find_solution();

    EXPECT_EQ(result.status, SearchStatus::SOLVED);
    EXPECT_EQ(result.plan.value().get_actions().size(), 5);
    EXPECT_EQ(result.plan.value().get_cost(), 5);

    const auto& astar_statistics = astar.get_algorithm_statistics();

    EXPECT_EQ(astar_statistics.get_num_generated_until_f_value().rbegin()->second, 1071);
    EXPECT_EQ(astar_statistics.get_num_expanded_until_f_value().rbegin()->second, 113);
}

/**
 * Tpp
 */

TEST(MimirTests, SearchAlgorithmsAStarGroundedBlindTppNumericTest)
{
    auto astar = GroundedAStarPlanner(fs::path(std::string(DATA_DIR) + "tpp/numeric/domain.pddl"),
                                      fs::path(std::string(DATA_DIR) + "tpp/numeric/test_problem.pddl"),
                                      HeuristicType::BLIND);
    auto result = astar.find_solution();

    EXPECT_EQ(result.status, SearchStatus::SOLVED);
    EXPECT_EQ(result.plan.value().get_actions().size(), 9);
    EXPECT_EQ(result.plan.value().get_cost(), 1833);

    const auto& astar_statistics = astar.get_algorithm_statistics();

    EXPECT_EQ(astar_statistics.get_num_generated_until_f_value().rbegin()->second, 2155);
    EXPECT_EQ(astar_statistics.get_num_expanded_until_f_value().rbegin()->second, 369);
}

TEST(MimirTests, SearchAlgorithmsAStarLiftedBlindTppNumericTest)
{
    auto astar = LiftedAStarPlanner(fs::path(std::string(DATA_DIR) + "tpp/numeric/domain.pddl"),
                                    fs::path(std::string(DATA_DIR) + "tpp/numeric/test_problem.pddl"),
                                    HeuristicType::BLIND);
    auto result = astar.find_solution();

    EXPECT_EQ(result.status, SearchStatus::SOLVED);
    EXPECT_EQ(result.plan.value().get_actions().size(), 9);
    EXPECT_EQ(result.plan.value().get_cost(), 1833);

    const auto& astar_statistics = astar.get_algorithm_statistics();

    EXPECT_EQ(astar_statistics.get_num_generated_until_f_value().rbegin()->second, 2155);
    EXPECT_EQ(astar_statistics.get_num_expanded_until_f_value().rbegin()->second, 369);
}

/**
 * Sugar
 */

TEST(MimirTests, SearchAlgorithmsAStarGroundedBlindZenotravelNumericTest)
{
    auto astar = GroundedAStarPlanner(fs::path(std::string(DATA_DIR) + "zenotravel/numeric/domain.pddl"),
                                      fs::path(std::string(DATA_DIR) + "zenotravel/numeric/test_problem.pddl"),
                                      HeuristicType::BLIND);
    auto result = astar.find_solution();

    EXPECT_EQ(result.status, SearchStatus::SOLVED);
    // EXPECT_EQ(result.plan.value().get_actions().size(), 9);  // plan lengths differ
    EXPECT_EQ(result.plan.value().get_cost(), 5952);

    const auto& astar_statistics = astar.get_algorithm_statistics();

    EXPECT_EQ(astar_statistics.get_num_generated_until_f_value().rbegin()->second, 1006);
    EXPECT_EQ(astar_statistics.get_num_expanded_until_f_value().rbegin()->second, 170);
}

TEST(MimirTests, SearchAlgorithmsAStarLiftedBlindZenotravelNumericTest)
{
    auto astar = LiftedAStarPlanner(fs::path(std::string(DATA_DIR) + "zenotravel/numeric/domain.pddl"),
                                    fs::path(std::string(DATA_DIR) + "zenotravel/numeric/test_problem.pddl"),
                                    HeuristicType::BLIND);
    auto result = astar.find_solution();

    EXPECT_EQ(result.status, SearchStatus::SOLVED);
    // EXPECT_EQ(result.plan.value().get_actions().size(), 9);  // plan lengths differ
    EXPECT_EQ(result.plan.value().get_cost(), 5952);

    const auto& astar_statistics = astar.get_algorithm_statistics();

    EXPECT_EQ(astar_statistics.get_num_generated_until_f_value().rbegin()->second, 1006);
    EXPECT_EQ(astar_statistics.get_num_expanded_until_f_value().rbegin()->second, 170);
}

}
