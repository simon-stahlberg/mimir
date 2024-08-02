#include "mimir/graphs/boost_adapter.hpp"

#include "mimir/datasets/state_space.hpp"

#include <boost/graph/graph_concepts.hpp>
#include <boost/graph/properties.hpp>
#include <gtest/gtest.h>

namespace mimir::tests
{

TEST(MimirTests, GraphsVertexListGraphTest)
{
    const auto domain_file = fs::path(std::string(DATA_DIR) + "gripper/domain.pddl");
    const auto problem_file = fs::path(std::string(DATA_DIR) + "gripper/p-2-0.pddl");
    const auto state_space = StateSpace::create(domain_file, problem_file).value();

    EXPECT_EQ(num_vertices(state_space.get_graph()), 28);

    auto [it, last] = vertices(state_space.get_graph());
    for (int i = 0; i < 28; ++i, ++it)
    {
        EXPECT_EQ(*it, i);
    }
    EXPECT_EQ(it, last);
}

TEST(MimirTests, GraphsIncidenceGraphTest)
{
    const auto domain_file = fs::path(std::string(DATA_DIR) + "gripper/domain.pddl");
    const auto problem_file = fs::path(std::string(DATA_DIR) + "gripper/p-2-0.pddl");
    const auto state_space = StateSpace::create(domain_file, problem_file).value();

    size_t transition_count = 0;
    for (auto [state_it, state_last] = vertices(state_space.get_graph()); state_it != state_last; ++state_it)
    {
        const auto& state_id = *state_it;
        size_t state_transition_count = 0;
        for (auto [out_it, out_last] = out_edges(state_id, state_space.get_graph()); out_it != out_last; ++out_it)
        {
            EXPECT_EQ(source(*out_it, state_space.get_graph()), state_id);
            transition_count++;
            state_transition_count++;
        }
        // Counting the number of transitions should give us the out degree.
        EXPECT_EQ(out_degree(*state_it, state_space.get_graph()), state_transition_count);
    }
    // Summing over the successors of each state should give the total number of transitions.
    EXPECT_EQ(transition_count, state_space.get_num_transitions());

    // possible actions:
    // pick(ball1, rooma, right), pick(ball1, rooma, left), pick(ball2, rooma, right), pick(ball2, rooma, left)
    // move(rooma, rooma), move(rooma, roomb)
    EXPECT_EQ(out_degree(state_space.get_initial_state(), state_space.get_graph()), 6);
}

TEST(MimirTests, GraphsStrongComponentsTest)
{
    {
        const auto domain_file = fs::path(std::string(DATA_DIR) + "gripper/domain.pddl");
        const auto problem_file = fs::path(std::string(DATA_DIR) + "gripper/p-2-0.pddl");
        const auto state_space = StateSpace::create(domain_file, problem_file).value();
        const auto [num_components, component_map] = strong_components(state_space.get_graph());
        EXPECT_EQ(num_components, 1);
        for (auto [it, last] = vertices(state_space.get_graph()); it != last; ++it)
        {
            EXPECT_EQ(component_map.at(*it), 0);
        }
    }
    {
        const auto domain_file = fs::path(std::string(DATA_DIR) + "spanner/domain.pddl");
        const auto problem_file = fs::path(std::string(DATA_DIR) + "spanner/test_problem.pddl");
        const auto state_space = StateSpace::create(domain_file, problem_file).value();
        const auto [num_components, component_map] = strong_components(state_space.get_graph());

        // Each state should have its own component.
        EXPECT_EQ(num_components, num_vertices(state_space.get_graph()));

        // Each component should only have one state.
        std::map<size_t, size_t> num_states_per_component;
        for (auto [it, last] = vertices(state_space.get_graph()); it != last; ++it)
        {
            num_states_per_component[component_map.at(*it)]++;
        }
        for (const auto& [key, val] : num_states_per_component)
        {
            EXPECT_EQ(val, 1);
        }
    }
}

TEST(MimirTests, GraphsDijkstraShortestPathTest)
{
    {
        const auto domain_file = fs::path(std::string(DATA_DIR) + "gripper/domain.pddl");
        const auto problem_file = fs::path(std::string(DATA_DIR) + "gripper/p-2-0.pddl");
        const auto state_space = StateSpace::create(domain_file, problem_file).value();
        const auto edge_costs = std::vector<double>(state_space.get_num_states(), 1);
        // const auto [predecessor_map, distance_map] = dijkstra_shortest_paths(state_space.get_graph(), edge_costs, state_space.get_initial_state());
    }
    /*
    {
        const auto domain_file = fs::path(std::string(DATA_DIR) + "spanner/domain.pddl");
        const auto problem_file = fs::path(std::string(DATA_DIR) + "spanner/test_problem.pddl");
        const auto state_space = StateSpace::create(domain_file, problem_file).value();
        const auto [num_components, component_map] = strong_components(state_space.get_graph());

        // Each state should have its own component.
        EXPECT_EQ(num_components, num_vertices(state_space.get_graph()));

        // Each component should only have one state.
        std::map<size_t, size_t> num_states_per_component;
        for (auto [it, last] = vertices(state_space.get_graph()); it != last; ++it)
        {
            num_states_per_component[component_map.at(*it)]++;
        }
        for (const auto& [key, val] : num_states_per_component)
        {
            EXPECT_EQ(val, 1);
        }
    }
    */
}

}
