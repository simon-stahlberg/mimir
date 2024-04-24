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
        auto state_repository = std::make_shared<SSG<SSGDispatcher<DenseStateTag>>>(m_parser.get_problem());
        auto successor_generator =
            (grounded) ?
                std::shared_ptr<IDynamicAAG> { std::make_shared<AAG<GroundedAAGDispatcher<DenseStateTag>>>(m_parser.get_problem(), m_parser.get_factories()) } :
                std::shared_ptr<IDynamicAAG> { std::make_shared<AAG<LiftedAAGDispatcher<DenseStateTag>>>(m_parser.get_problem(), m_parser.get_factories()) };
        auto event_handler = std::make_shared<MinimalEventHandler>();
        m_algorithm = std::make_unique<BrFsAlgorithm>(m_parser.get_problem(), m_parser.get_factories(), state_repository, successor_generator, event_handler);
    }

    std::tuple<SearchStatus, Plan> find_solution()
    {
        auto action_view_list = std::vector<ConstView<ActionDispatcher<StateReprTag>>> {};
        const auto status = m_algorithm->find_solution(action_view_list);
        return std::make_tuple(status, to_plan(action_view_list));
    }
};

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

}
