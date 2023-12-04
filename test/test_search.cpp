#include "../include/mimir/formalism/domain.hpp"
#include "../include/mimir/formalism/problem.hpp"
#include "../include/mimir/generators/complete_state_space.hpp"
#include "../include/mimir/generators/successor_generator.hpp"
#include "../include/mimir/generators/successor_generator_factory.hpp"
#include "../include/mimir/pddl/parsers.hpp"
#include "../include/mimir/search/breadth_first_search.hpp"

// Test instances

#include "instances/blocks/domain.hpp"
#include "instances/blocks/problem.hpp"
#include "instances/gripper/domain.hpp"
#include "instances/gripper/problem.hpp"
#include "instances/spanner/domain.hpp"
#include "instances/spanner/problem.hpp"
#include "instances/spider/domain.hpp"
#include "instances/spider/problem.hpp"

#include <gtest/gtest.h>
#include <sstream>
#include <string>

namespace test
{
    class SearchTest : public testing::TestWithParam<std::tuple<std::string, std::string, int32_t*, int32_t, int32_t>>
    {
    };

    TEST_P(SearchTest, Parameterized)
    {
        const auto domain_text = std::get<0>(GetParam());
        const auto problem_text = std::get<1>(GetParam());
        const auto expanded_array = std::get<2>(GetParam());
        const auto expanded_length = std::get<3>(GetParam());
        const auto plan_length = std::get<4>(GetParam());

        std::istringstream domain_stream(domain_text);
        std::istringstream problem_stream(problem_text);

        const auto domain = mimir::parsers::DomainParser::parse(domain_stream);
        const auto problem = mimir::parsers::ProblemParser::parse(domain, "", problem_stream);

        const auto successor_generator = mimir::planners::create_sucessor_generator(problem, mimir::planners::SuccessorGeneratorType::GROUNDED);

        const auto state_space = mimir::planners::create_complete_state_space(problem, successor_generator);
        auto search = mimir::planners::create_breadth_first_search(problem, successor_generator);

        search->register_handler(
            [&search, &expanded_array, &expanded_length]()
            {
                const auto statistics = search->get_statistics();
                ASSERT_TRUE(statistics.count("expanded"));
                ASSERT_TRUE(statistics.count("max_depth"));
                const auto expanded = std::get<int32_t>(statistics.at("expanded"));
                const auto depth = std::get<int32_t>(statistics.at("max_depth"));

                if (depth < expanded_length)
                {
                    ASSERT_EQ(expanded, expanded_array[depth]);
                }
                else
                {
                    search->abort();
                }
            });

        mimir::formalism::ActionList plan;
        const auto result = search->plan(plan);
        ASSERT_EQ(result, mimir::planners::SearchResult::SOLVED);
        ASSERT_EQ(plan.size(), plan_length);
    }

    INSTANTIATE_TEST_SUITE_P(
        ParamTest,
        SearchTest,
        testing::Values(std::make_tuple(blocks::domain, blocks::problem, blocks::bfs_result, blocks::bfs_length, blocks::bfs_plan_length),
                        std::make_tuple(gripper::domain, gripper::problem, gripper::bfs_result, gripper::bfs_length, gripper::bfs_plan_length),
                        std::make_tuple(spanner::domain, spanner::problem, spanner::bfs_result, spanner::bfs_length, spanner::bfs_plan_length),
                        std::make_tuple(spider::domain, spider::problem, spider::bfs_result, spider::bfs_length, spider::bfs_plan_length)));
}  // namespace test
