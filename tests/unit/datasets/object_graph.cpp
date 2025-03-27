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

#include "mimir/datasets/generalized_color_function.hpp"
#include "mimir/datasets/generalized_state_space.hpp"
#include "mimir/formalism/problem.hpp"
#include "mimir/graphs/algorithms/nauty.hpp"

#include <gtest/gtest.h>
#include <unordered_set>

using namespace mimir::datasets;
using namespace mimir::formalism;
using namespace mimir::graphs;

namespace mimir::tests
{

TEST(MimirTests, DataSetsObjectGraphDenseTest)
{
    const auto domain_file = fs::path(std::string(DATA_DIR) + "gripper/domain.pddl");
    const auto problem_file = fs::path(std::string(DATA_DIR) + "gripper/p-2-0.pddl");

    auto options = StateSpaceImpl::Options();
    options.symmetry_pruning = false;
    const auto context = search::SearchContext(domain_file, problem_file);
    const auto state_space = StateSpaceImpl::create(context, options);
    const auto color_function = GeneralizedColorFunctionImpl(context.get_problem());

    auto certificates = std::unordered_set<nauty::Certificate, loki::Hash<nauty::Certificate>, loki::EqualTo<nauty::Certificate>> {};

    for (const auto& vertex : state_space.value()->get_graph().get_vertices())
    {
        const auto state = get_state(vertex);
        const auto& problem = get_problem(vertex);

        const auto object_graph = create_object_graph(state, *problem, color_function);

        auto certificate = compute_certificate(nauty::DenseGraph(object_graph));

        certificates.insert(std::move(certificate));
    }

    EXPECT_EQ(certificates.size(), 12);
}

TEST(MimirTests, DataSetsObjectGraphSparseTest)
{
    const auto domain_file = fs::path(std::string(DATA_DIR) + "gripper/domain.pddl");
    const auto problem_file = fs::path(std::string(DATA_DIR) + "gripper/p-2-0.pddl");

    auto options = StateSpaceImpl::Options();
    options.symmetry_pruning = false;
    const auto context = search::SearchContext(domain_file, problem_file);
    const auto state_space = StateSpaceImpl::create(context, options);
    const auto color_function = GeneralizedColorFunctionImpl(context.get_problem());

    auto certificates = std::unordered_set<nauty::Certificate, loki::Hash<nauty::Certificate>, loki::EqualTo<nauty::Certificate>> {};

    for (const auto& vertex : state_space.value()->get_graph().get_vertices())
    {
        const auto state = get_state(vertex);
        const auto& problem = get_problem(vertex);

        const auto object_graph = create_object_graph(state, *problem, color_function);

        auto certificate = compute_certificate(nauty::SparseGraph(object_graph));

        certificates.insert(std::move(certificate));
    }

    EXPECT_EQ(certificates.size(), 12);
}

}
