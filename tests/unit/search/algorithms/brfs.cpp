#include "mimir/formalism/declarations.hpp"
#include "mimir/formalism/parser.hpp"
#include "mimir/search/algorithms.hpp"
#include "mimir/search/event_handlers.hpp"

#include <gtest/gtest.h>

namespace mimir::tests
{

/// @brief Instantiate a BrFs
class BrFsPlanner
{
private:
    PDDLParser m_parser;

    std::unique_ptr<IAlgorithm> m_algorithm;

public:
    BrFsPlanner(const fs::path& domain_file, const fs::path& problem_file, bool grounded) :
        m_parser(PDDLParser(domain_file, problem_file)),
        m_algorithm(nullptr)
    {
        auto successor_generator =
            (grounded) ?
                std::shared_ptr<IDynamicAAG> { std::make_shared<AAG<GroundedAAGDispatcher<DenseStateTag>>>(m_parser.get_problem(), m_parser.get_factories()) } :
                std::shared_ptr<IDynamicAAG> { std::make_shared<AAG<LiftedAAGDispatcher<DenseStateTag>>>(m_parser.get_problem(), m_parser.get_factories()) };
        auto event_handler = std::make_shared<MinimalEventHandler>();
        m_algorithm = std::make_unique<BrFsAlgorithm>(successor_generator, event_handler);
    }

    std::tuple<SearchStatus, Plan> find_solution()
    {
        auto action_view_list = GroundActionList {};
        const auto status = m_algorithm->find_solution(action_view_list);
        return std::make_tuple(status, to_plan(action_view_list));
    }
};

/**
 * Airport
 */

/**
 * Barman
 */
TEST(MimirTests, SearchAlgorithmsBrFSGroundedBarmanTest)
{
    auto brfs = BrFsPlanner(fs::path(std::string(DATA_DIR) + "barman/domain.pddl"), fs::path(std::string(DATA_DIR) + "barman/test_problem.pddl"), true);
    const auto [search_status, plan] = brfs.find_solution();
    EXPECT_EQ(search_status, SearchStatus::SOLVED);
    EXPECT_EQ(plan.get_actions().size(), 11);
}

TEST(MimirTests, SearchAlgorithmsBrFSLiftedBarmanTest)
{
    auto brfs = BrFsPlanner(fs::path(std::string(DATA_DIR) + "barman/domain.pddl"), fs::path(std::string(DATA_DIR) + "barman/test_problem.pddl"), false);
    const auto [search_status, plan] = brfs.find_solution();
    EXPECT_EQ(search_status, SearchStatus::SOLVED);
    EXPECT_EQ(plan.get_actions().size(), 11);
}

/**
 * Blocks 3 ops
 */

TEST(MimirTests, SearchAlgorithmsBrFSGroundedBlocks3opsTest)
{
    auto brfs = BrFsPlanner(fs::path(std::string(DATA_DIR) + "blocks_3/domain.pddl"), fs::path(std::string(DATA_DIR) + "blocks_3/test_problem.pddl"), true);
    const auto [search_status, plan] = brfs.find_solution();
    EXPECT_EQ(search_status, SearchStatus::SOLVED);
    EXPECT_EQ(plan.get_actions().size(), 4);
}

TEST(MimirTests, SearchAlgorithmsBrFSLiftedBlocks3opsTest)
{
    auto brfs = BrFsPlanner(fs::path(std::string(DATA_DIR) + "blocks_3/domain.pddl"), fs::path(std::string(DATA_DIR) + "blocks_3/test_problem.pddl"), false);
    const auto [search_status, plan] = brfs.find_solution();
    EXPECT_EQ(search_status, SearchStatus::SOLVED);
    EXPECT_EQ(plan.get_actions().size(), 4);
}

/**
 * Blocks 4 ops
 */

TEST(MimirTests, SearchAlgorithmsBrFSGroundedBlocks4opsTest)
{
    auto brfs = BrFsPlanner(fs::path(std::string(DATA_DIR) + "blocks_4/domain.pddl"), fs::path(std::string(DATA_DIR) + "blocks_4/test_problem.pddl"), true);
    const auto [search_status, plan] = brfs.find_solution();
    EXPECT_EQ(search_status, SearchStatus::SOLVED);
    EXPECT_EQ(plan.get_actions().size(), 4);
}

TEST(MimirTests, SearchAlgorithmsBrFSLiftedBlocks4opsTest)
{
    auto brfs = BrFsPlanner(fs::path(std::string(DATA_DIR) + "blocks_4/domain.pddl"), fs::path(std::string(DATA_DIR) + "blocks_4/test_problem.pddl"), false);
    const auto [search_status, plan] = brfs.find_solution();
    EXPECT_EQ(search_status, SearchStatus::SOLVED);
    EXPECT_EQ(plan.get_actions().size(), 4);
}

/**
 * Childsnack
 */

TEST(MimirTests, SearchAlgorithmsBrFSGroundedChildsnackTest)
{
    auto brfs = BrFsPlanner(fs::path(std::string(DATA_DIR) + "childsnack/domain.pddl"), fs::path(std::string(DATA_DIR) + "childsnack/test_problem.pddl"), true);
    const auto [search_status, plan] = brfs.find_solution();
    EXPECT_EQ(search_status, SearchStatus::SOLVED);
    EXPECT_EQ(plan.get_actions().size(), 4);
}

TEST(MimirTests, SearchAlgorithmsBrFSLiftedChildsnackTest)
{
    auto brfs =
        BrFsPlanner(fs::path(std::string(DATA_DIR) + "childsnack/domain.pddl"), fs::path(std::string(DATA_DIR) + "childsnack/test_problem.pddl"), false);
    const auto [search_status, plan] = brfs.find_solution();
    EXPECT_EQ(search_status, SearchStatus::SOLVED);
    EXPECT_EQ(plan.get_actions().size(), 4);
}

/**
 * Delivery
 */

TEST(MimirTests, SearchAlgorithmsBrFSGroundedDeliveryTest)
{
    auto brfs = BrFsPlanner(fs::path(std::string(DATA_DIR) + "delivery/domain.pddl"), fs::path(std::string(DATA_DIR) + "delivery/test_problem.pddl"), true);
    const auto [search_status, plan] = brfs.find_solution();
    EXPECT_EQ(search_status, SearchStatus::SOLVED);
    EXPECT_EQ(plan.get_actions().size(), 4);
}

TEST(MimirTests, SearchAlgorithmsBrFSLiftedDeliveryTest)
{
    auto brfs = BrFsPlanner(fs::path(std::string(DATA_DIR) + "delivery/domain.pddl"), fs::path(std::string(DATA_DIR) + "delivery/test_problem.pddl"), false);
    const auto [search_status, plan] = brfs.find_solution();
    EXPECT_EQ(search_status, SearchStatus::SOLVED);
    EXPECT_EQ(plan.get_actions().size(), 4);
}

/**
 * Driverlog
 */

TEST(MimirTests, SearchAlgorithmsBrFSGroundedDriverlogTest)
{
    auto brfs = BrFsPlanner(fs::path(std::string(DATA_DIR) + "driverlog/domain.pddl"), fs::path(std::string(DATA_DIR) + "driverlog/test_problem.pddl"), true);
    const auto [search_status, plan] = brfs.find_solution();
    EXPECT_EQ(search_status, SearchStatus::SOLVED);
    EXPECT_EQ(plan.get_actions().size(), 9);
}

TEST(MimirTests, SearchAlgorithmsBrFSLiftedDriverlogTest)
{
    auto brfs = BrFsPlanner(fs::path(std::string(DATA_DIR) + "driverlog/domain.pddl"), fs::path(std::string(DATA_DIR) + "driverlog/test_problem.pddl"), false);
    const auto [search_status, plan] = brfs.find_solution();
    EXPECT_EQ(search_status, SearchStatus::SOLVED);
    EXPECT_EQ(plan.get_actions().size(), 9);
}

/**
 * Ferry
 */

TEST(MimirTests, SearchAlgorithmsBrFSGroundedFerryTest)
{
    /*
       TODO: negative preconditions cause us troubles in the delete relaxation.
    */
    auto brfs = BrFsPlanner(fs::path(std::string(DATA_DIR) + "ferry/domain.pddl"), fs::path(std::string(DATA_DIR) + "ferry/test_problem.pddl"), true);
    const auto [search_status, plan] = brfs.find_solution();
    EXPECT_EQ(search_status, SearchStatus::SOLVED);
    EXPECT_EQ(plan.get_actions().size(), 7);
}

TEST(MimirTests, SearchAlgorithmsBrFSLiftedFerryTest)
{
    auto brfs = BrFsPlanner(fs::path(std::string(DATA_DIR) + "ferry/domain.pddl"), fs::path(std::string(DATA_DIR) + "ferry/test_problem.pddl"), false);
    const auto [search_status, plan] = brfs.find_solution();
    EXPECT_EQ(search_status, SearchStatus::SOLVED);
    EXPECT_EQ(plan.get_actions().size(), 7);
}

/**
 * Grid
 */

TEST(MimirTests, SearchAlgorithmsBrFSGroundedGridTest)
{
    auto brfs = BrFsPlanner(fs::path(std::string(DATA_DIR) + "grid/domain.pddl"), fs::path(std::string(DATA_DIR) + "grid/test_problem.pddl"), true);
    const auto [search_status, plan] = brfs.find_solution();
    EXPECT_EQ(search_status, SearchStatus::SOLVED);
    EXPECT_EQ(plan.get_actions().size(), 4);
}

TEST(MimirTests, SearchAlgorithmsBrFSLiftedGridTest)
{
    auto brfs = BrFsPlanner(fs::path(std::string(DATA_DIR) + "grid/domain.pddl"), fs::path(std::string(DATA_DIR) + "grid/test_problem.pddl"), false);
    const auto [search_status, plan] = brfs.find_solution();
    EXPECT_EQ(search_status, SearchStatus::SOLVED);
    EXPECT_EQ(plan.get_actions().size(), 4);
}

/**
 * Gripper
 */

TEST(MimirTests, SearchAlgorithmsBrFSGroundedGripperTest)
{
    auto brfs = BrFsPlanner(fs::path(std::string(DATA_DIR) + "gripper/domain.pddl"), fs::path(std::string(DATA_DIR) + "gripper/test_problem.pddl"), true);
    const auto [search_status, plan] = brfs.find_solution();
    EXPECT_EQ(search_status, SearchStatus::SOLVED);
    EXPECT_EQ(plan.get_actions().size(), 3);
}

TEST(MimirTests, SearchAlgorithmsBrFSLiftedGripperTest)
{
    auto brfs = BrFsPlanner(fs::path(std::string(DATA_DIR) + "gripper/domain.pddl"), fs::path(std::string(DATA_DIR) + "gripper/test_problem.pddl"), false);
    const auto [search_status, plan] = brfs.find_solution();
    EXPECT_EQ(search_status, SearchStatus::SOLVED);
    EXPECT_EQ(plan.get_actions().size(), 3);
}

/**
 * Hiking
 */

TEST(MimirTests, SearchAlgorithmsBrFSGroundedHikingTest)
{
    auto brfs = BrFsPlanner(fs::path(std::string(DATA_DIR) + "hiking/domain.pddl"), fs::path(std::string(DATA_DIR) + "hiking/test_problem.pddl"), true);
    const auto [search_status, plan] = brfs.find_solution();
    EXPECT_EQ(search_status, SearchStatus::SOLVED);
    EXPECT_EQ(plan.get_actions().size(), 4);
}

TEST(MimirTests, SearchAlgorithmsBrFSLiftedHikingTest)
{
    auto brfs = BrFsPlanner(fs::path(std::string(DATA_DIR) + "hiking/domain.pddl"), fs::path(std::string(DATA_DIR) + "hiking/test_problem.pddl"), false);
    const auto [search_status, plan] = brfs.find_solution();
    EXPECT_EQ(search_status, SearchStatus::SOLVED);
    EXPECT_EQ(plan.get_actions().size(), 4);
}

/**
 * Logistics
 */

TEST(MimirTests, SearchAlgorithmsBrFSGroundedLogisticsTest)
{
    auto brfs = BrFsPlanner(fs::path(std::string(DATA_DIR) + "logistics/domain.pddl"), fs::path(std::string(DATA_DIR) + "logistics/test_problem.pddl"), true);
    const auto [search_status, plan] = brfs.find_solution();
    EXPECT_EQ(search_status, SearchStatus::SOLVED);
    EXPECT_EQ(plan.get_actions().size(), 4);
}

TEST(MimirTests, SearchAlgorithmsBrFSLiftedLogisticsTest)
{
    auto brfs = BrFsPlanner(fs::path(std::string(DATA_DIR) + "logistics/domain.pddl"), fs::path(std::string(DATA_DIR) + "logistics/test_problem.pddl"), false);
    const auto [search_status, plan] = brfs.find_solution();
    EXPECT_EQ(search_status, SearchStatus::SOLVED);
    EXPECT_EQ(plan.get_actions().size(), 4);
}

/**
 * Miconic
 */

TEST(MimirTests, SearchAlgorithmsBrFSGroundedMiconicTest)
{
    auto brfs = BrFsPlanner(fs::path(std::string(DATA_DIR) + "miconic/domain.pddl"), fs::path(std::string(DATA_DIR) + "miconic/test_problem.pddl"), true);
    const auto [search_status, plan] = brfs.find_solution();
    EXPECT_EQ(search_status, SearchStatus::SOLVED);
    EXPECT_EQ(plan.get_actions().size(), 5);
}

TEST(MimirTests, SearchAlgorithmsBrFSLiftedMiconicTest)
{
    auto brfs = BrFsPlanner(fs::path(std::string(DATA_DIR) + "miconic/domain.pddl"), fs::path(std::string(DATA_DIR) + "miconic/test_problem.pddl"), false);
    const auto [search_status, plan] = brfs.find_solution();
    EXPECT_EQ(search_status, SearchStatus::SOLVED);
    EXPECT_EQ(plan.get_actions().size(), 5);
}

/**
 * Miconic-fulladl
 */

/*
TEST(MimirTests, SearchAlgorithmsBrFSGroundedMiconicFulladlTest)
{
    auto brfs = BrFsPlanner(fs::path(std::string(DATA_DIR) + "miconic-fulladl/domain.pddl"),
                            fs::path(std::string(DATA_DIR) + "miconic-fulladl/test_problem.pddl"),
                            true);
    const auto [search_status, plan] = brfs.find_solution();
    EXPECT_EQ(search_status, SearchStatus::SOLVED);
    EXPECT_EQ(plan.get_actions().size(), 7);
}
*/

TEST(MimirTests, SearchAlgorithmsBrFSLiftedMiconicFulladlTest)
{
    auto brfs = BrFsPlanner(fs::path(std::string(DATA_DIR) + "miconic-fulladl/domain.pddl"),
                            fs::path(std::string(DATA_DIR) + "miconic-fulladl/test_problem.pddl"),
                            false);
    const auto [search_status, plan] = brfs.find_solution();
    EXPECT_EQ(search_status, SearchStatus::SOLVED);
    EXPECT_EQ(plan.get_actions().size(), 7);
}

/**
 * Miconic-simpleadl
 */

TEST(MimirTests, SearchAlgorithmsBrFSGroundedMiconicSimpleadlTest)
{
    auto brfs = BrFsPlanner(fs::path(std::string(DATA_DIR) + "miconic-simpleadl/domain.pddl"),
                            fs::path(std::string(DATA_DIR) + "miconic-simpleadl/test_problem.pddl"),
                            true);
    const auto [search_status, plan] = brfs.find_solution();
    EXPECT_EQ(search_status, SearchStatus::SOLVED);
    EXPECT_EQ(plan.get_actions().size(), 4);
}

TEST(MimirTests, SearchAlgorithmsBrFSLiftedMiconicSimpleadlTest)
{
    auto brfs = BrFsPlanner(fs::path(std::string(DATA_DIR) + "miconic-simpleadl/domain.pddl"),
                            fs::path(std::string(DATA_DIR) + "miconic-simpleadl/test_problem.pddl"),
                            false);
    const auto [search_status, plan] = brfs.find_solution();
    EXPECT_EQ(search_status, SearchStatus::SOLVED);
    EXPECT_EQ(plan.get_actions().size(), 4);
}

/**
 * Reward
 */

TEST(MimirTests, SearchAlgorithmsBrFSGroundedRewardTest)
{
    auto brfs = BrFsPlanner(fs::path(std::string(DATA_DIR) + "reward/domain.pddl"), fs::path(std::string(DATA_DIR) + "reward/test_problem.pddl"), true);
    const auto [search_status, plan] = brfs.find_solution();
    EXPECT_EQ(search_status, SearchStatus::SOLVED);
    EXPECT_EQ(plan.get_actions().size(), 4);
}

TEST(MimirTests, SearchAlgorithmsBrFSLiftedRewardTest)
{
    auto brfs = BrFsPlanner(fs::path(std::string(DATA_DIR) + "reward/domain.pddl"), fs::path(std::string(DATA_DIR) + "reward/test_problem.pddl"), false);
    const auto [search_status, plan] = brfs.find_solution();
    EXPECT_EQ(search_status, SearchStatus::SOLVED);
    EXPECT_EQ(plan.get_actions().size(), 4);
}

/**
 * Rovers
 */

TEST(MimirTests, SearchAlgorithmsBrFSGroundedRoversTest)
{
    auto brfs = BrFsPlanner(fs::path(std::string(DATA_DIR) + "rovers/domain.pddl"), fs::path(std::string(DATA_DIR) + "rovers/test_problem.pddl"), true);
    const auto [search_status, plan] = brfs.find_solution();
    EXPECT_EQ(search_status, SearchStatus::SOLVED);
    EXPECT_EQ(plan.get_actions().size(), 4);
}

TEST(MimirTests, SearchAlgorithmsBrFSLiftedRoversTest)
{
    auto brfs = BrFsPlanner(fs::path(std::string(DATA_DIR) + "rovers/domain.pddl"), fs::path(std::string(DATA_DIR) + "rovers/test_problem.pddl"), false);
    const auto [search_status, plan] = brfs.find_solution();
    EXPECT_EQ(search_status, SearchStatus::SOLVED);
    EXPECT_EQ(plan.get_actions().size(), 4);
}

/**
 * Satellite
 */

TEST(MimirTests, SearchAlgorithmsBrFSGroundedSatelliteTest)
{
    auto brfs = BrFsPlanner(fs::path(std::string(DATA_DIR) + "satellite/domain.pddl"), fs::path(std::string(DATA_DIR) + "satellite/test_problem.pddl"), true);
    const auto [search_status, plan] = brfs.find_solution();
    EXPECT_EQ(search_status, SearchStatus::SOLVED);
    EXPECT_EQ(plan.get_actions().size(), 7);
}

TEST(MimirTests, SearchAlgorithmsBrFSLiftedSatelliteTest)
{
    auto brfs = BrFsPlanner(fs::path(std::string(DATA_DIR) + "satellite/domain.pddl"), fs::path(std::string(DATA_DIR) + "satellite/test_problem.pddl"), false);
    const auto [search_status, plan] = brfs.find_solution();
    EXPECT_EQ(search_status, SearchStatus::SOLVED);
    EXPECT_EQ(plan.get_actions().size(), 7);
}

/**
 * Schedule
 */

/**
 * Spanner
 */

TEST(MimirTests, SearchAlgorithmsBrFSGroundedSpannerTest)
{
    auto brfs = BrFsPlanner(fs::path(std::string(DATA_DIR) + "spanner/domain.pddl"), fs::path(std::string(DATA_DIR) + "spanner/test_problem.pddl"), true);
    const auto [search_status, plan] = brfs.find_solution();
    EXPECT_EQ(search_status, SearchStatus::SOLVED);
    EXPECT_EQ(plan.get_actions().size(), 4);
}

TEST(MimirTests, SearchAlgorithmsBrFSLiftedSpannerTest)
{
    auto brfs = BrFsPlanner(fs::path(std::string(DATA_DIR) + "spanner/domain.pddl"), fs::path(std::string(DATA_DIR) + "spanner/test_problem.pddl"), false);
    const auto [search_status, plan] = brfs.find_solution();
    EXPECT_EQ(search_status, SearchStatus::SOLVED);
    EXPECT_EQ(plan.get_actions().size(), 4);
}

/**
 * Transport
 */

TEST(MimirTests, SearchAlgorithmsBrFSGroundedTransportTest)
{
    auto brfs = BrFsPlanner(fs::path(std::string(DATA_DIR) + "transport/domain.pddl"), fs::path(std::string(DATA_DIR) + "transport/test_problem.pddl"), true);
    const auto [search_status, plan] = brfs.find_solution();
    EXPECT_EQ(search_status, SearchStatus::SOLVED);
    EXPECT_EQ(plan.get_actions().size(), 5);
}

TEST(MimirTests, SearchAlgorithmsBrFSLiftedTransportTest)
{
    auto brfs = BrFsPlanner(fs::path(std::string(DATA_DIR) + "transport/domain.pddl"), fs::path(std::string(DATA_DIR) + "transport/test_problem.pddl"), false);
    const auto [search_status, plan] = brfs.find_solution();
    EXPECT_EQ(search_status, SearchStatus::SOLVED);
    EXPECT_EQ(plan.get_actions().size(), 5);
}

/**
 * Visitall
 */

TEST(MimirTests, SearchAlgorithmsBrFSGroundedVisitallTest)
{
    auto brfs = BrFsPlanner(fs::path(std::string(DATA_DIR) + "visitall/domain.pddl"), fs::path(std::string(DATA_DIR) + "visitall/test_problem.pddl"), true);
    const auto [search_status, plan] = brfs.find_solution();
    EXPECT_EQ(search_status, SearchStatus::SOLVED);
    EXPECT_EQ(plan.get_actions().size(), 8);
}

TEST(MimirTests, SearchAlgorithmsBrFSLiftedVisitallTest)
{
    auto brfs = BrFsPlanner(fs::path(std::string(DATA_DIR) + "visitall/domain.pddl"), fs::path(std::string(DATA_DIR) + "visitall/test_problem.pddl"), false);
    const auto [search_status, plan] = brfs.find_solution();
    EXPECT_EQ(search_status, SearchStatus::SOLVED);
    EXPECT_EQ(plan.get_actions().size(), 8);
}

/**
 * Woodworking
 */

TEST(MimirTests, SearchAlgorithmsBrFSGroundedWoodworkingTest)
{
    auto brfs =
        BrFsPlanner(fs::path(std::string(DATA_DIR) + "woodworking/domain.pddl"), fs::path(std::string(DATA_DIR) + "woodworking/test_problem.pddl"), true);
    const auto [search_status, plan] = brfs.find_solution();
    EXPECT_EQ(search_status, SearchStatus::SOLVED);
    EXPECT_EQ(plan.get_actions().size(), 2);
}

TEST(MimirTests, SearchAlgorithmsBrFSLiftedWoodworkingTest)
{
    auto brfs =
        BrFsPlanner(fs::path(std::string(DATA_DIR) + "woodworking/domain.pddl"), fs::path(std::string(DATA_DIR) + "woodworking/test_problem.pddl"), false);
    const auto [search_status, plan] = brfs.find_solution();
    EXPECT_EQ(search_status, SearchStatus::SOLVED);
    EXPECT_EQ(plan.get_actions().size(), 2);
}

}
