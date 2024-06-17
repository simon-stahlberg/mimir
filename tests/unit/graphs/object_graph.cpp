#include "mimir/graphs/object_graph.hpp"

#include "mimir/datasets/state_space.hpp"

#include <gtest/gtest.h>

namespace mimir::tests
{

TEST(MimirTests, GraphsObjectGraphTest)
{
    const auto domain_file = fs::path(std::string(DATA_DIR) + "gripper/domain.pddl");
    const auto problem_file = fs::path(std::string(DATA_DIR) + "gripper/test_problem.pddl");

    const auto state_space = StateSpaceImpl::create(domain_file, problem_file, 10000, 10000);

    auto object_graph_factory = ObjectGraphFactory(state_space->get_problem());

    for (const auto& state : state_space->get_states())
    {
        // std::cout << std::make_tuple(state_space->get_aag()->get_problem(), state, std::cref(state_space->get_aag()->get_pddl_factories())) << std::endl;

        const auto& object_graph = object_graph_factory.create(state, state_space->get_factories());

        std::cout << object_graph << std::endl;

        auto nauty_graph = nauty_wrapper::Graph(object_graph.get_digraph().get_num_vertices());
        object_graph.get_digraph().to_nauty_graph(nauty_graph);
        const auto certificate = nauty_graph.compute_certificate(object_graph.get_vertex_partitioning());

        std::cout << certificate << std::endl;
    }
}
}
