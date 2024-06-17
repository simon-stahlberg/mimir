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

        // std::cout << object_graph << std::endl;
    }
}
}
