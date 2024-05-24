#include "mimir/formalism/formalism.hpp"
#include "mimir/search/algorithms.hpp"
#include "mimir/search/algorithms/event_handlers.hpp"
#include "mimir/search/heuristics.hpp"
#include "mimir/search/plan.hpp"

#include <gtest/gtest.h>

namespace mimir::tests
{

/// @brief Instantiate a AStar
class AStarBlindPlanner
{
private:
    PDDLParser m_parser;

    std::unique_ptr<IAlgorithm> m_algorithm;

public:
    AStarBlindPlanner(const fs::path& domain_file, const fs::path& problem_file, bool grounded) :
        m_parser(PDDLParser(domain_file, problem_file)),
        m_algorithm(nullptr)
    {
        auto successor_generator =
            (grounded) ?
                std::shared_ptr<IDynamicAAG> { std::make_shared<AAG<GroundedAAGDispatcher<DenseStateTag>>>(m_parser.get_problem(), m_parser.get_factories()) } :
                std::shared_ptr<IDynamicAAG> { std::make_shared<AAG<LiftedAAGDispatcher<DenseStateTag>>>(m_parser.get_problem(), m_parser.get_factories()) };
        auto blind_heuristic = std::make_shared<Heuristic<HeuristicDispatcher<BlindTag, DenseStateTag>>>();
        m_algorithm = std::make_unique<AStarAlgorithm>(successor_generator, blind_heuristic);
    }

    std::tuple<SearchStatus, Plan> find_solution()
    {
        auto action_view_list = GroundActionList {};
        const auto status = m_algorithm->find_solution(action_view_list);
        return std::make_tuple(status, to_plan(action_view_list));
    }
};

/**
 * Gripper
 */
TEST(MimirTests, SearchAlgorithmsAStarBlindGroundedGripperTest)
{
    auto brfs = AStarBlindPlanner(fs::path(std::string(DATA_DIR) + "gripper/domain.pddl"), fs::path(std::string(DATA_DIR) + "gripper/test_problem.pddl"), true);
    const auto [search_status, plan] = brfs.find_solution();
    // not implemented
    // EXPECT_EQ(search_status, SearchStatus::SOLVED);
    // EXPECT_EQ(plan.get_actions().size(), 3);
}

TEST(MimirTests, SearchAlgorithmsAStarBlindLiftedGripperTest)
{
    auto brfs =
        AStarBlindPlanner(fs::path(std::string(DATA_DIR) + "gripper/domain.pddl"), fs::path(std::string(DATA_DIR) + "gripper/test_problem.pddl"), false);
    const auto [search_status, plan] = brfs.find_solution();
    // not implemented
    // EXPECT_EQ(search_status, SearchStatus::SOLVED);
    // EXPECT_EQ(plan.get_actions().size(), 3);
}

}
