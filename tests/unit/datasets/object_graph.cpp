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

#include "mimir/common/equal_to.hpp"
#include "mimir/common/hash.hpp"
#include "mimir/datasets/state_space.hpp"
#include "mimir/formalism/problem.hpp"
#include "mimir/graphs/algorithms/nauty.hpp"
#include "mimir/search/search_context.hpp"

#include <gtest/gtest.h>
#include <unordered_set>

using namespace mimir::datasets;
using namespace mimir::formalism;
using namespace mimir::graphs;

namespace mimir::tests
{

TEST(MimirTests, DataSetsObjectGraphSparseTest)
{
    const auto domain_file = fs::path(std::string(DATA_DIR) + "gripper/domain.pddl");
    const auto problem_file = fs::path(std::string(DATA_DIR) + "gripper/p-2-0.pddl");

    auto options = state_space::Options();
    options.symmetry_pruning = false;
    const auto context = search::SearchContextImpl::create(domain_file, problem_file);
    const auto state_space_result = StateSpaceImpl::create(context, options);

    auto certificates = UnorderedSet<nauty::SparseGraph> {};

    for (const auto& vertex : state_space_result->first->get_graph().get_vertices())
    {
        certificates.insert(nauty::SparseGraph(create_object_graph(get_state(vertex), *get_problem(vertex))).canonize());
    }

    EXPECT_EQ(certificates.size(), 12);
}

}
