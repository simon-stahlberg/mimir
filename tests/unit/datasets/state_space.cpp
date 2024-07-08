#include "mimir/datasets/state_space.hpp"

#include "mimir/datasets/boost_adapter.hpp"

#include <boost/graph/graph_concepts.hpp>
#include <boost/graph/properties.hpp>
#include <gtest/gtest.h>

namespace mimir::tests
{

TEST(MimirTests, DatasetsStateSpaceCreateTest)
{
    const auto domain_file = fs::path(std::string(DATA_DIR) + "gripper/domain.pddl");
    const auto problem_file = fs::path(std::string(DATA_DIR) + "gripper/p-2-0.pddl");

    const auto state_space = StateSpace::create(domain_file, problem_file).value();

    EXPECT_EQ(state_space.get_num_states(), 28);
    EXPECT_EQ(state_space.get_num_transitions(), 104);
    EXPECT_EQ(state_space.get_num_goal_states(), 2);
    EXPECT_EQ(state_space.get_num_deadend_states(), 0);
}

TEST(MimirTests, DatasetsStateSpaceCreateParallelTest)
{
    const auto domain_file = fs::path(std::string(DATA_DIR) + "gripper/domain.pddl");
    const auto problem_file_1 = fs::path(std::string(DATA_DIR) + "gripper/p-1-0.pddl");
    const auto problem_file_2 = fs::path(std::string(DATA_DIR) + "gripper/p-2-0.pddl");
    const auto problem_files = std::vector<fs::path> { problem_file_1, problem_file_2 };

    const auto state_spaces = StateSpace::create(domain_file, problem_files);

    EXPECT_EQ(state_spaces.size(), 2);
}

TEST(MimirTests, DatasetsStateSpaceVertexListGraphTest)
{
    const auto domain_file = fs::path(std::string(DATA_DIR) + "gripper/domain.pddl");
    const auto problem_file = fs::path(std::string(DATA_DIR) + "gripper/p-2-0.pddl");
    const auto state_space = StateSpace::create(domain_file, problem_file).value();

    EXPECT_EQ(num_vertices(state_space), 28);

    auto [it, last] = vertices(state_space);
    for (int i = 0; i < 28; ++i, ++it)
    {
        EXPECT_EQ(*it, i);
    }
    EXPECT_EQ(it, last);
}

TEST(MimirTests, DatasetsStateSpaceIncidenceGraphTest)
{
    const auto domain_file = fs::path(std::string(DATA_DIR) + "gripper/domain.pddl");
    const auto problem_file = fs::path(std::string(DATA_DIR) + "gripper/p-2-0.pddl");
    const auto state_space = StateSpace::create(domain_file, problem_file).value();

    size_t transition_count = 0;
    for (auto [state_it, state_last] = vertices(state_space); state_it != state_last; ++state_it)
    {
        const auto& state_id = *state_it;
        size_t state_transition_count = 0;
        for (auto [out_it, out_last] = out_edges(state_id, state_space); out_it != out_last; ++out_it)
        {
            EXPECT_EQ(source(*out_it, state_space), state_id);
            transition_count++;
            state_transition_count++;
        }
        // Counting the number of transitions should give us the out degree.
        EXPECT_EQ(out_degree(*state_it, state_space), state_transition_count);
    }
    // Summing over the successors of each state should give the total number of transitions.
    EXPECT_EQ(transition_count, state_space.get_num_transitions());

    // possible actions:
    // pick(ball1, rooma, right), pick(ball1, rooma, left), pick(ball2, rooma, right), pick(ball2, rooma, left)
    // move(rooma, rooma), move(rooma, roomb)
    EXPECT_EQ(out_degree(state_space.get_initial_state(), state_space), 6);
}

TEST(MimirTests, DatasetsStateSpaceStrongComponentsTest)
{
    {
        const auto domain_file = fs::path(std::string(DATA_DIR) + "gripper/domain.pddl");
        const auto problem_file = fs::path(std::string(DATA_DIR) + "gripper/p-2-0.pddl");
        const auto state_space = StateSpace::create(domain_file, problem_file).value();
        const auto [num_components, component_map] = strong_components(state_space);
        EXPECT_EQ(num_components, 1);
        for (auto [it, last] = vertices(state_space); it != last; ++it)
        {
            EXPECT_EQ(component_map.at(*it), 0);
        }
    }
    {
        const auto domain_file = fs::path(std::string(DATA_DIR) + "spanner/domain.pddl");
        const auto problem_file = fs::path(std::string(DATA_DIR) + "spanner/test_problem.pddl");
        const auto state_space = StateSpace::create(domain_file, problem_file).value();
        const auto [num_components, component_map] = strong_components(state_space);

        // Each state should have its own component.
        EXPECT_EQ(num_components, num_vertices(state_space));

        // Each component should only have one state.
        std::map<size_t, size_t> num_states_per_component;
        for (auto [it, last] = vertices(state_space); it != last; ++it)
        {
            num_states_per_component[component_map.at(*it)]++;
        }
        for (const auto& [key, val] : num_states_per_component)
        {
            EXPECT_EQ(val, 1);
        }
    }
}
}
