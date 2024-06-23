#include "mimir/graphs/tuple_graph.hpp"

#include <gtest/gtest.h>

namespace mimir::tests
{

TEST(MimirTests, GraphsTupleGraphTest)
{
    const auto domain_file = fs::path(std::string(DATA_DIR) + "gripper/domain.pddl");
    const auto problem_file = fs::path(std::string(DATA_DIR) + "gripper/test_problem.pddl");
    PDDLParser parser(domain_file, problem_file);

    const auto state_space = std::make_shared<StateSpace>(std::move(StateSpace::create(domain_file, problem_file).value()));

    auto tuple_graph_factory = TupleGraphFactory(state_space, 2, false);

    for (const auto& state : state_space->get_states())
    {
        // std::cout << std::make_tuple(state_space->get_aag()->get_problem(), state, std::cref(state_space->get_aag()->get_pddl_factories())) << std::endl;

        const auto& tuple_graph = tuple_graph_factory.create(state);

        // std::cout << tuple_graph << std::endl;
    }
}
}
