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

#include "mimir/search/heuristics/h2.hpp"

#include "mimir/formalism/parser.hpp"
#include "mimir/search/grounders/lifted.hpp"
#include "mimir/search/search_context.hpp"
#include "mimir/search/state_repository.hpp"
#include "mimir/search/state.hpp"

#include <gtest/gtest.h>

using namespace mimir::search;
using namespace mimir::formalism;

namespace mimir::tests
{
    class InitialHeuristicTest : public testing::TestWithParam<std::pair<std::string, double>>
    {
    };

    TEST_P(InitialHeuristicTest, SearchHeuristicsH2InitialStateTest)
    {
        const auto [domain_name, expected_initial_heuristic] = GetParam();
        const auto domain_file = fs::path(std::string(DATA_DIR) + domain_name + "/domain.pddl");
        const auto problem_file = fs::path(std::string(DATA_DIR) + domain_name + "/test_problem.pddl");

        auto parser = Parser(domain_file);
        const auto domain = parser.get_domain();
        const auto problem = parser.parse_problem(problem_file);

        const auto search_options = SearchContextImpl::Options(SearchContextImpl::GroundedOptions());
        const auto search_context = SearchContextImpl::create(problem, search_options);
        const auto initial_state = search_context->get_state_repository()->get_or_create_initial_state().first;

        const auto grounder = LiftedGrounder(problem);
        const auto h2 = H2HeuristicImpl::create(grounder);

        EXPECT_EQ(h2->compute_heuristic(initial_state), expected_initial_heuristic);
    }

    INSTANTIATE_TEST_SUITE_P(
        MimirTests,
        InitialHeuristicTest,
        testing::Values(
            std::make_pair("blocks_4", 4.0),
            std::make_pair("childsnack", 4.0),
            std::make_pair("grid", 4.0),
            std::make_pair("gripper", 3.0),
            std::make_pair("logistics", 3.0),
            std::make_pair("miconic", 4.0),
            std::make_pair("reward", 4.0),
            std::make_pair("rovers", 4.0),
            std::make_pair("satellite", 5.0),
            std::make_pair("spanner", 4.0)
        ));
}
