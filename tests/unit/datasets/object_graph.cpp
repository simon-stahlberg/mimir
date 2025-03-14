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

#include "mimir/datasets/object_graph.hpp"

#include "mimir/algorithms/nauty.hpp"
#include "mimir/datasets/generalized_color_function.hpp"
#include "mimir/datasets/generalized_state_space.hpp"
#include "mimir/formalism/problem.hpp"

#include <gtest/gtest.h>
#include <unordered_set>

namespace mimir::tests
{

TEST(MimirTests, DataSetsObjectGraphDenseTest)
{
    const auto domain_file = fs::path(std::string(DATA_DIR) + "gripper/domain.pddl");
    const auto problem_file = fs::path(std::string(DATA_DIR) + "gripper/p-2-0.pddl");

    auto options = datasets::GeneralizedStateSpace::Options();
    options.problem_options.symmetry_pruning = false;
    const auto context = GeneralizedSearchContext(domain_file, std::vector<fs::path> { problem_file });
    const auto problem_class_state_space = datasets::GeneralizedStateSpace(context, options);
    const auto color_function = datasets::GeneralizedColorFunction(context.get_generalized_problem());

    auto certificates = std::unordered_set<nauty_wrapper::Certificate, loki::Hash<nauty_wrapper::Certificate>, loki::EqualTo<nauty_wrapper::Certificate>> {};

    for (const auto& vertex : problem_class_state_space.get_graph().get_vertices())
    {
        const auto state = get_state(problem_class_state_space.get_problem_vertex(vertex));
        const auto problem_index = get_problem_index(vertex);
        const auto& problem = problem_class_state_space.get_generalized_search_context().get_generalized_problem().get_problems().at(problem_index);

        const auto object_graph = create_object_graph(state, *problem, color_function);

        auto certificate = nauty_wrapper::DenseGraph(object_graph).compute_certificate();

        certificates.insert(std::move(certificate));
    }

    EXPECT_EQ(certificates.size(), 12);
}

TEST(MimirTests, DataSetsObjectGraphSparseTest)
{
    const auto domain_file = fs::path(std::string(DATA_DIR) + "gripper/domain.pddl");
    const auto problem_file = fs::path(std::string(DATA_DIR) + "gripper/p-2-0.pddl");

    auto options = datasets::GeneralizedStateSpace::Options();
    options.problem_options.symmetry_pruning = false;
    const auto context = GeneralizedSearchContext(domain_file, std::vector<fs::path> { problem_file });
    const auto problem_class_state_space = datasets::GeneralizedStateSpace(context, options);
    const auto color_function = datasets::GeneralizedColorFunction(context.get_generalized_problem());

    auto certificates = std::unordered_set<nauty_wrapper::Certificate, loki::Hash<nauty_wrapper::Certificate>, loki::EqualTo<nauty_wrapper::Certificate>> {};

    for (const auto& vertex : problem_class_state_space.get_graph().get_vertices())
    {
        const auto state = get_state(problem_class_state_space.get_problem_vertex(vertex));
        const auto problem_index = get_problem_index(vertex);
        const auto& problem = problem_class_state_space.get_generalized_search_context().get_generalized_problem().get_problems().at(problem_index);

        const auto object_graph = create_object_graph(state, *problem, color_function);

        auto certificate = nauty_wrapper::SparseGraph(object_graph).compute_certificate();

        certificates.insert(std::move(certificate));
    }

    EXPECT_EQ(certificates.size(), 12);
}

}
