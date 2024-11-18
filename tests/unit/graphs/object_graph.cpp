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
    auto certificates = std::unordered_set<nauty_wrapper::Certificate> {};

    for (const auto& vertex : state_space.get_graph().get_vertices())
    {
        const auto state = get_state(vertex);

        // std::cout << std::make_tuple(state_space->get_applicable_action_generator()->get_problem(), state,
        // std::cref(state_space->get_applicable_action_generator()->get_pddl_repositories())) << std::endl;

        const auto object_graph = create_object_graph(color_function, *state_space.get_pddl_repositories(), state_space.get_problem(), state);

        // std::cout << object_graph << std::endl;

        auto certificate = nauty_wrapper::DenseGraph(object_graph).compute_certificate();

        certificates.insert(std::move(certificate));
    }

    EXPECT_EQ(state_space.get_num_vertices(), 28);
    EXPECT_EQ(certificates.size(), 12);
}

TEST(MimirTests, GraphsObjectGraphSparseTest)
{
    const auto domain_file = fs::path(std::string(DATA_DIR) + "gripper/domain.pddl");
    const auto problem_file = fs::path(std::string(DATA_DIR) + "gripper/p-2-0.pddl");

    const auto state_space = StateSpace::create(domain_file, problem_file).value();

    const auto color_function = ProblemColorFunction(state_space.get_problem());
    auto certificates = std::unordered_set<nauty_wrapper::Certificate> {};

    for (const auto& vertex : state_space.get_graph().get_vertices())
    {
        const auto state = get_state(vertex);
        // std::cout << std::make_tuple(state_space->get_applicable_action_generator()->get_problem(), state,
        // std::cref(state_space->get_applicable_action_generator()->get_pddl_repositories())) << std::endl;

        const auto object_graph = create_object_graph(color_function, *state_space.get_pddl_repositories(), state_space.get_problem(), state);

        // std::cout << object_graph << std::endl;

        auto certificate = nauty_wrapper::SparseGraph(object_graph).compute_certificate();

        certificates.insert(std::move(certificate));
    }

    EXPECT_EQ(state_space.get_num_vertices(), 28);
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
        bool prune(Index, Object) const override { return true; };
        bool prune(Index, GroundAtom<Static>) const override { return true; };
        bool prune(Index, GroundAtom<Fluent>) const override { return true; };
        bool prune(Index, GroundAtom<Derived>) const override { return true; };
        bool prune(Index, GroundLiteral<Static>) const override { return true; }
        bool prune(Index, GroundLiteral<Fluent>) const override { return true; }
        bool prune(Index, GroundLiteral<Derived>) const override { return true; }
    };

    const auto color_function = ProblemColorFunction(state_space.get_problem());
    auto certificates = std::unordered_set<nauty_wrapper::Certificate> {};

    for (const auto& vertex : state_space.get_graph().get_vertices())
    {
        const auto state = get_state(vertex);
        // std::cout << std::make_tuple(state_space->get_applicable_action_generator()->get_problem(), state,
        // std::cref(state_space->get_applicable_action_generator()->get_pddl_repositories())) << std::endl;

        const auto object_graph =
            create_object_graph(color_function, *state_space.get_pddl_repositories(), state_space.get_problem(), state, 0, true, PruneAllObjects());

        // std::cout << object_graph << std::endl;

        auto certificate = nauty_wrapper::SparseGraph(object_graph).compute_certificate();

        certificates.insert(std::move(certificate));
    }

    EXPECT_EQ(state_space.get_num_vertices(), 28);
    EXPECT_EQ(certificates.size(), 1);
}

}
