/*
 * Copyright (C) 2023 Dominik Drexler and Simon Stahlberg
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "mimir/graphs/object_graph.hpp"

#include "mimir/algorithms/nauty.hpp"
#include "mimir/common/hash.hpp"
#include "mimir/datasets/state_space.hpp"
#include "mimir/graphs/certificate.hpp"

#include <gtest/gtest.h>
#include <unordered_set>

namespace mimir::tests
{

TEST(MimirTests, GraphsObjectGraphDenseTest)
{
    const auto domain_file = fs::path(std::string(DATA_DIR) + "gripper/domain.pddl");
    const auto problem_file = fs::path(std::string(DATA_DIR) + "gripper/p-2-0.pddl");

    const auto state_space = StateSpace::create(domain_file, problem_file).value();

    const auto color_function = ProblemColorFunction(state_space.get_problem());
    auto certificates = std::unordered_set<Certificate> {};

    for (const auto& vertex : state_space.get_graph().get_vertices())
    {
        const auto state = get_state(vertex);

        // std::cout << std::make_tuple(state_space->get_aag()->get_problem(), state, std::cref(state_space->get_aag()->get_pddl_factories())) << std::endl;

        const auto object_graph = create_object_graph(color_function, *state_space.get_pddl_factories(), state_space.get_problem(), state);

        // std::cout << object_graph << std::endl;

        auto certificate = Certificate(object_graph.get_num_vertices(),
                                       object_graph.get_num_edges(),
                                       nauty_wrapper::DenseGraph(object_graph).compute_certificate(),
                                       compute_sorted_vertex_colors(object_graph));

        certificates.insert(std::move(certificate));
    }

    EXPECT_EQ(state_space.get_num_states(), 28);
    EXPECT_EQ(certificates.size(), 12);
}

TEST(MimirTests, GraphsObjectGraphSparseTest)
{
    const auto domain_file = fs::path(std::string(DATA_DIR) + "gripper/domain.pddl");
    const auto problem_file = fs::path(std::string(DATA_DIR) + "gripper/p-2-0.pddl");

    const auto state_space = StateSpace::create(domain_file, problem_file).value();

    const auto color_function = ProblemColorFunction(state_space.get_problem());
    auto certificates = std::unordered_set<Certificate> {};

    for (const auto& vertex : state_space.get_graph().get_vertices())
    {
        const auto state = get_state(vertex);
        // std::cout << std::make_tuple(state_space->get_aag()->get_problem(), state, std::cref(state_space->get_aag()->get_pddl_factories())) << std::endl;

        const auto object_graph = create_object_graph(color_function, *state_space.get_pddl_factories(), state_space.get_problem(), state);

        // std::cout << object_graph << std::endl;

        auto certificate = Certificate(object_graph.get_num_vertices(),
                                       object_graph.get_num_edges(),
                                       nauty_wrapper::SparseGraph(object_graph).compute_certificate(),
                                       compute_sorted_vertex_colors(object_graph));

        certificates.insert(std::move(certificate));
    }

    EXPECT_EQ(state_space.get_num_states(), 28);
    EXPECT_EQ(certificates.size(), 12);
}

TEST(MimirTests, GraphsObjectGraphPruningTest)
{
    const auto domain_file = fs::path(std::string(DATA_DIR) + "gripper/domain.pddl");
    const auto problem_file = fs::path(std::string(DATA_DIR) + "gripper/p-2-0.pddl");
    const auto state_space = StateSpace::create(domain_file, problem_file).value();

    class PruneAllObjects : public ObjectGraphPruningStrategy
    {
    public:
        bool prune(StateIndex, Object) const override { return true; };
        bool prune(StateIndex, GroundAtom<Static>) const override { return true; };
        bool prune(StateIndex, GroundAtom<Fluent>) const override { return true; };
        bool prune(StateIndex, GroundAtom<Derived>) const override { return true; };
        bool prune(StateIndex, GroundLiteral<Static>) const override { return true; }
        bool prune(StateIndex, GroundLiteral<Fluent>) const override { return true; }
        bool prune(StateIndex, GroundLiteral<Derived>) const override { return true; }
    };

    const auto color_function = ProblemColorFunction(state_space.get_problem());
    auto certificates = std::unordered_set<Certificate> {};

    for (const auto& vertex : state_space.get_graph().get_vertices())
    {
        const auto state = get_state(vertex);
        // std::cout << std::make_tuple(state_space->get_aag()->get_problem(), state, std::cref(state_space->get_aag()->get_pddl_factories())) << std::endl;

        const auto object_graph =
            create_object_graph(color_function, *state_space.get_pddl_factories(), state_space.get_problem(), state, 0, true, PruneAllObjects());

        // std::cout << object_graph << std::endl;

        auto certificate = Certificate(object_graph.get_num_vertices(),
                                       object_graph.get_num_edges(),
                                       nauty_wrapper::SparseGraph(object_graph).compute_certificate(),
                                       compute_sorted_vertex_colors(object_graph));

        certificates.insert(std::move(certificate));
    }

    EXPECT_EQ(state_space.get_num_states(), 28);
    EXPECT_EQ(certificates.size(), 1);
}

}
