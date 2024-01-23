#include "../include/mimir/formalism/domain.hpp"
#include "../include/mimir/formalism/problem.hpp"
#include "../include/mimir/pddl/parsers.hpp"

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
    void assert_domain_parse(const mimir::formalism::DomainDescription& domain, const test::DomainParseResult& expect)
    {
        ASSERT_EQ(domain->action_schemas.size(), expect.num_action_schemas);
        ASSERT_EQ(domain->predicates.size(), expect.num_predicates);
        ASSERT_EQ(domain->constants.size(), expect.num_constants);
        ASSERT_EQ(domain->types.size(), expect.num_types);
    }

    void assert_problem_parse(const mimir::formalism::ProblemDescription& problem, const test::ProblemParseResult& expect)
    {
        ASSERT_EQ(problem->objects.size(), expect.num_objects);
        ASSERT_EQ(problem->initial.size(), expect.num_initial);
        ASSERT_EQ(problem->goal.size(), expect.num_goal);
    }

    class ParseTest : public testing::TestWithParam<std::tuple<std::string, DomainParseResult, std::string, ProblemParseResult>>
    {
    };

    TEST(Parse, MultipleTimes)
    {
        for (std::size_t i = 0; i < 5; ++i)
        {
            std::istringstream domain_stream(blocks::domain);
            std::istringstream problem_stream(blocks::problem);

            const auto domain = mimir::parsers::DomainParser::parse(domain_stream);
            const auto problem = mimir::parsers::ProblemParser::parse(domain, "", problem_stream);

            assert_domain_parse(domain, blocks::domain_parse_result);
            assert_problem_parse(problem, blocks::problem_parse_result);
        }
    }

    TEST_P(ParseTest, Parameterized)
    {
        const auto domain_text = std::get<0>(GetParam());
        const auto domain_result = std::get<1>(GetParam());
        const auto problem_text = std::get<2>(GetParam());
        const auto problem_result = std::get<3>(GetParam());

        std::istringstream domain_stream(domain_text);
        std::istringstream problem_stream(problem_text);

        const auto domain = mimir::parsers::DomainParser::parse(domain_stream);
        const auto problem = mimir::parsers::ProblemParser::parse(domain, "", problem_stream);

        assert_domain_parse(domain, domain_result);
        assert_problem_parse(problem, problem_result);
    }

    INSTANTIATE_TEST_SUITE_P(ParamTest,
                             ParseTest,
                             testing::Values(std::make_tuple(blocks::domain, blocks::domain_parse_result, blocks::problem, blocks::problem_parse_result),
                                             std::make_tuple(gripper::domain, gripper::domain_parse_result, gripper::problem, gripper::problem_parse_result),
                                             std::make_tuple(spanner::domain, spanner::domain_parse_result, spanner::problem, spanner::problem_parse_result),
                                             std::make_tuple(spider::domain, spider::domain_parse_result, spider::problem, spider::problem_parse_result)));
}  // namespace test
