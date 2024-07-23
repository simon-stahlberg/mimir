#include "mimir/datasets/faithful_abstraction.hpp"

#include <gtest/gtest.h>

namespace mimir::tests
{

TEST(MimirTests, DatasetsFaithfulAbstractionCreateTest)
{
    const auto domain_file = fs::path(std::string(DATA_DIR) + "gripper/domain.pddl");
    const auto problem_file = fs::path(std::string(DATA_DIR) + "gripper/p-2-0.pddl");

    const auto abstraction = FaithfulAbstraction::create(domain_file, problem_file).value();

    EXPECT_EQ(abstraction.get_num_states(), 12);
    EXPECT_EQ(abstraction.get_num_transitions(), 36);
    EXPECT_EQ(abstraction.get_num_goal_states(), 2);
    EXPECT_EQ(abstraction.get_num_deadend_states(), 0);
}

TEST(MimirTests, DatasetsFaithfulAbstractionCreateParallelTest)
{
    const auto domain_file = fs::path(std::string(DATA_DIR) + "gripper/domain.pddl");
    const auto problem_file_1 = fs::path(std::string(DATA_DIR) + "gripper/p-1-0.pddl");
    const auto problem_file_2 = fs::path(std::string(DATA_DIR) + "gripper/p-2-0.pddl");
    const auto problem_files = std::vector<fs::path> { problem_file_1, problem_file_2 };

    const auto abstractions = FaithfulAbstraction::create(domain_file, problem_files);

    EXPECT_EQ(abstractions.size(), 2);

    // Reduced from 8 to 6 abstract states.
    EXPECT_EQ(abstractions.at(0).get_num_states(), 6);
    // Reduced from 28 to 12 abstract states.
    EXPECT_EQ(abstractions.at(1).get_num_states(), 12);
}

TEST(MimirTests, DatasetsFaithfulAbstractionCreateSingleSCCPruningTest)
{
    const auto domain_file = fs::path(std::string(DATA_DIR) + "delivery/domain.pddl");
    const auto problem_file = fs::path(std::string(DATA_DIR) + "delivery/test_problem.pddl");
    const auto parser = PDDLParser(domain_file, problem_file);
    const auto aag = std::make_shared<GroundedAAG>(parser.get_problem(), parser.get_factories());
    const auto ssg = std::make_shared<SuccessorStateGenerator>(aag);

    auto options = FaithfulAbstractionOptions();
    options.pruning_strategy = ObjectGraphPruningStrategyEnum::StaticScc;
    const auto faithful_abstraction = FaithfulAbstraction::create(parser.get_problem(), parser.get_factories(), aag, ssg, options);
}

TEST(MimirTests, DatasetsFaithfulAbstractionCreateMultiSCCPruningTest)
{
    const auto domain_file = fs::path(std::string(DATA_DIR) + "spanner/domain.pddl");
    const auto problem_file = fs::path(std::string(DATA_DIR) + "spanner/test_problem.pddl");
    const auto parser = PDDLParser(domain_file, problem_file);
    const auto aag = std::make_shared<GroundedAAG>(parser.get_problem(), parser.get_factories());
    const auto ssg = std::make_shared<SuccessorStateGenerator>(aag);

    auto options = FaithfulAbstractionOptions();
    options.pruning_strategy = ObjectGraphPruningStrategyEnum::StaticScc;
    const auto faithful_abstraction = FaithfulAbstraction::create(parser.get_problem(), parser.get_factories(), aag, ssg, options);
}

}
