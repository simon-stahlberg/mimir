#include "mimir/graphs/tuple_graph.hpp"

#include <gtest/gtest.h>

namespace mimir::tests
{

TEST(MimirTests, GraphsTupleGraphTest)
{
    const auto domain_file = fs::path(std::string(DATA_DIR) + "gripper/domain.pddl");
    const auto problem_file = fs::path(std::string(DATA_DIR) + "gripper/p-1-0.pddl");
    PDDLParser parser(domain_file, problem_file);

    const auto state_space = std::make_shared<StateSpace>(std::move(StateSpace::create(domain_file, problem_file).value()));

    EXPECT_EQ(state_space->get_num_states(), 8);

    auto tuple_graph_factory_0 = TupleGraphFactory(state_space, 0, false);
    auto tuple_graph_factory_1 = TupleGraphFactory(state_space, 1, false);
    auto tuple_graph_factory_2 = TupleGraphFactory(state_space, 2, false);
    auto tuple_graph_factory_0_pruned = TupleGraphFactory(state_space, 0, true);
    auto tuple_graph_factory_1_pruned = TupleGraphFactory(state_space, 1, true);
    auto tuple_graph_factory_2_pruned = TupleGraphFactory(state_space, 2, true);

    auto tuple_graphs_0 = TupleGraphList {};
    auto tuple_graphs_1 = TupleGraphList {};
    auto tuple_graphs_2 = TupleGraphList {};
    auto tuple_graphs_0_pruned = TupleGraphList {};
    auto tuple_graphs_1_pruned = TupleGraphList {};
    auto tuple_graphs_2_pruned = TupleGraphList {};

    for (const auto& state : state_space->get_states())
    {
        // std::cout << std::make_tuple(state_space->get_aag()->get_problem(), state, std::cref(*state_space->get_aag()->get_pddl_factories())) << std::endl;

        tuple_graphs_0.push_back(tuple_graph_factory_0.create(state));
        tuple_graphs_1.push_back(tuple_graph_factory_1.create(state));
        tuple_graphs_2.push_back(tuple_graph_factory_2.create(state));

        tuple_graphs_0_pruned.push_back(tuple_graph_factory_0_pruned.create(state));
        tuple_graphs_1_pruned.push_back(tuple_graph_factory_1_pruned.create(state));
        tuple_graphs_2_pruned.push_back(tuple_graph_factory_2_pruned.create(state));

        // std::cout << tuple_graph << std::endl;
    }

    // std::cout << tuple_graphs_0.at(0) << std::endl;
    // std::cout << tuple_graphs_1.at(0) << std::endl;
    // std::cout << tuple_graphs_2.at(0) << std::endl;

    // std::cout << tuple_graphs_0.at(7) << std::endl;
    // std::cout << tuple_graphs_1.at(7) << std::endl;
    // std::cout << tuple_graphs_2.at(7) << std::endl;

    EXPECT_EQ(tuple_graphs_0.at(0).get_digraph().get_num_vertices(), 4);
    EXPECT_EQ(tuple_graphs_0.at(0).get_digraph().get_num_edges(), 3);
    EXPECT_EQ(tuple_graphs_0.at(0).get_vertices_grouped_by_distance().size(), 2);
    EXPECT_EQ(tuple_graphs_0.at(0).get_states_grouped_by_distance().size(), 2);
    EXPECT_EQ(tuple_graphs_1.at(0).get_digraph().get_num_vertices(), 8);
    EXPECT_EQ(tuple_graphs_1.at(0).get_digraph().get_num_edges(), 15);
    EXPECT_EQ(tuple_graphs_1.at(0).get_vertices_grouped_by_distance().size(), 2);
    EXPECT_EQ(tuple_graphs_1.at(0).get_states_grouped_by_distance().size(), 2);
    EXPECT_EQ(tuple_graphs_2.at(0).get_digraph().get_num_vertices(), 28);
    EXPECT_EQ(tuple_graphs_2.at(0).get_digraph().get_num_edges(), 128);
    EXPECT_EQ(tuple_graphs_2.at(0).get_vertices_grouped_by_distance().size(), 5);
    EXPECT_EQ(tuple_graphs_2.at(0).get_states_grouped_by_distance().size(), 5);

    EXPECT_EQ(tuple_graphs_0_pruned.at(0).get_digraph().get_num_vertices(), 4);
    EXPECT_EQ(tuple_graphs_0_pruned.at(0).get_digraph().get_num_edges(), 3);
    EXPECT_EQ(tuple_graphs_0_pruned.at(0).get_vertices_grouped_by_distance().size(), 2);
    EXPECT_EQ(tuple_graphs_0_pruned.at(0).get_states_grouped_by_distance().size(), 2);
    EXPECT_EQ(tuple_graphs_1_pruned.at(0).get_digraph().get_num_vertices(), 4);
    EXPECT_EQ(tuple_graphs_1_pruned.at(0).get_digraph().get_num_edges(), 3);
    EXPECT_EQ(tuple_graphs_1_pruned.at(0).get_vertices_grouped_by_distance().size(), 2);
    EXPECT_EQ(tuple_graphs_1_pruned.at(0).get_states_grouped_by_distance().size(), 2);
    EXPECT_EQ(tuple_graphs_2_pruned.at(0).get_digraph().get_num_vertices(), 8);
    EXPECT_EQ(tuple_graphs_2_pruned.at(0).get_digraph().get_num_edges(), 8);
    EXPECT_EQ(tuple_graphs_2_pruned.at(0).get_vertices_grouped_by_distance().size(), 5);
    EXPECT_EQ(tuple_graphs_2_pruned.at(0).get_states_grouped_by_distance().size(), 5);

    EXPECT_EQ(tuple_graphs_0.at(7).get_digraph().get_num_vertices(), 3);
    EXPECT_EQ(tuple_graphs_0.at(7).get_digraph().get_num_edges(), 2);
    EXPECT_EQ(tuple_graphs_0.at(7).get_vertices_grouped_by_distance().size(), 2);
    EXPECT_EQ(tuple_graphs_0.at(7).get_states_grouped_by_distance().size(), 2);
    EXPECT_EQ(tuple_graphs_1.at(7).get_digraph().get_num_vertices(), 9);
    EXPECT_EQ(tuple_graphs_1.at(7).get_digraph().get_num_edges(), 15);
    EXPECT_EQ(tuple_graphs_1.at(7).get_vertices_grouped_by_distance().size(), 3);
    EXPECT_EQ(tuple_graphs_1.at(7).get_states_grouped_by_distance().size(), 3);
    EXPECT_EQ(tuple_graphs_2.at(7).get_digraph().get_num_vertices(), 28);
    EXPECT_EQ(tuple_graphs_2.at(7).get_digraph().get_num_edges(), 131);
    EXPECT_EQ(tuple_graphs_2.at(7).get_vertices_grouped_by_distance().size(), 4);
    EXPECT_EQ(tuple_graphs_2.at(7).get_states_grouped_by_distance().size(), 4);

    EXPECT_EQ(tuple_graphs_0_pruned.at(7).get_digraph().get_num_vertices(), 3);
    EXPECT_EQ(tuple_graphs_0_pruned.at(7).get_digraph().get_num_edges(), 2);
    EXPECT_EQ(tuple_graphs_0_pruned.at(7).get_vertices_grouped_by_distance().size(), 2);
    EXPECT_EQ(tuple_graphs_0_pruned.at(7).get_states_grouped_by_distance().size(), 2);
    EXPECT_EQ(tuple_graphs_1_pruned.at(7).get_digraph().get_num_vertices(), 5);
    EXPECT_EQ(tuple_graphs_1_pruned.at(7).get_digraph().get_num_edges(), 4);
    EXPECT_EQ(tuple_graphs_1_pruned.at(7).get_vertices_grouped_by_distance().size(), 3);
    EXPECT_EQ(tuple_graphs_1_pruned.at(7).get_states_grouped_by_distance().size(), 3);
    EXPECT_EQ(tuple_graphs_2_pruned.at(7).get_digraph().get_num_vertices(), 8);
    EXPECT_EQ(tuple_graphs_2_pruned.at(7).get_digraph().get_num_edges(), 8);
    EXPECT_EQ(tuple_graphs_2_pruned.at(7).get_vertices_grouped_by_distance().size(), 4);
    EXPECT_EQ(tuple_graphs_2_pruned.at(7).get_states_grouped_by_distance().size(), 4);

    /* Test computation of admissible chain */
    const auto& fluent_goal_conditions = state_space->get_problem()->get_goal_condition<Fluent>();
    EXPECT_EQ(fluent_goal_conditions.size(), 1);

    // const auto goal_atom = fluent_goal_conditions.at(0)->get_atom();

    // TODO need brfs implementation

    // EXPECT_EQ(tuple_graphs_0.at(0).compute_admissible_chain({ goal_atom }, {}), std::nullopt);
    // EXPECT_EQ(tuple_graphs_1.at(0).compute_admissible_chain({ goal_atom }, {}), std::nullopt);
    // EXPECT_NE(tuple_graphs_2.at(0).compute_admissible_chain({ goal_atom }, {}), std::nullopt);
}

}
