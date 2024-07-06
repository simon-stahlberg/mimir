#include "mimir/graphs/object_graph.hpp"

#include "mimir/algorithms/nauty.hpp"
#include "mimir/datasets/state_space.hpp"
#include "mimir/graphs/certificate.hpp"

#include <gtest/gtest.h>
#include <loki/loki.hpp>
#include <unordered_set>

namespace mimir::tests
{

TEST(MimirTests, GraphsObjectGraphDenseTest)
{
    const auto domain_file = fs::path(std::string(DATA_DIR) + "gripper/domain.pddl");
    const auto problem_file = fs::path(std::string(DATA_DIR) + "gripper/p-2-0.pddl");

    const auto state_space = StateSpace::create(domain_file, problem_file).value();

    auto object_graph_factory = ObjectGraphFactory(state_space.get_pddl_parser()->get_problem(), state_space.get_pddl_parser()->get_factories());

    auto nauty_graph_factory = nauty_wrapper::DenseGraphFactory();

    auto certificates = std::unordered_set<Certificate, loki::Hash<Certificate>, loki::EqualTo<Certificate>> {};

    for (const auto& state : state_space.get_states())
    {
        // std::cout << std::make_tuple(state_space->get_aag()->get_problem(), state, std::cref(state_space->get_aag()->get_pddl_factories())) << std::endl;

        const auto& object_graph = object_graph_factory.create(state);

        // std::cout << object_graph << std::endl;

        auto certificate = Certificate(nauty_graph_factory.create_from_digraph(object_graph.get_digraph()).compute_certificate(object_graph.get_partitioning()),
                                       object_graph.get_sorted_vertex_colors());

        certificates.insert(std::move(certificate));
    }

    EXPECT_EQ(state_space.get_states().size(), 28);
    EXPECT_EQ(certificates.size(), 12);
}

TEST(MimirTests, GraphsObjectGraphSparseTest)
{
    const auto domain_file = fs::path(std::string(DATA_DIR) + "gripper/domain.pddl");
    const auto problem_file = fs::path(std::string(DATA_DIR) + "gripper/p-2-0.pddl");

    const auto state_space = StateSpace::create(domain_file, problem_file).value();

    auto object_graph_factory = ObjectGraphFactory(state_space.get_pddl_parser()->get_problem(), state_space.get_pddl_parser()->get_factories(), true);

    auto nauty_graph_factory = nauty_wrapper::SparseGraphFactory();

    auto certificates = std::unordered_set<Certificate, loki::Hash<Certificate>, loki::EqualTo<Certificate>> {};

    for (const auto& state : state_space.get_states())
    {
        // std::cout << std::make_tuple(state_space->get_aag()->get_problem(), state, std::cref(state_space->get_aag()->get_pddl_factories())) << std::endl;

        const auto& object_graph = object_graph_factory.create(state);

        // std::cout << object_graph << std::endl;

        auto certificate = Certificate(nauty_graph_factory.create_from_digraph(object_graph.get_digraph()).compute_certificate(object_graph.get_partitioning()),
                                       object_graph.get_sorted_vertex_colors());

        certificates.insert(std::move(certificate));
    }

    EXPECT_EQ(state_space.get_states().size(), 28);
    EXPECT_EQ(certificates.size(), 12);
}
}
