#include "../mimir/formalism/domain.hpp"
#include "../mimir/formalism/problem.hpp"
#include "../mimir/pddl/parsers.hpp"

// PDDL files encoded as static strings named "domain_<DOMAIN>" or "problem_<DOMAIN>"
// The expected results are stored as structs named "domain_<DOMAIN>_<TYPE>_result" and "problem_<DOMAIN>_<TYPE>_result"
// Where <TYPE> is a type of result, e.g., parsing or expanding.

#include "instances/blocks/domain.hpp"
#include "instances/blocks/problem.hpp"
#include "instances/gripper/domain.hpp"
#include "instances/gripper/problem.hpp"
#include "instances/spider/domain.hpp"
#include "instances/spider/problem.hpp"

#include <gtest/gtest.h>
#include <sstream>
#include <string>

namespace test
{
    void assert_domain_parse(const formalism::DomainDescription& domain, const test::DomainParseResult& expect)
    {
        ASSERT_EQ(domain->action_schemas.size(), expect.num_action_schemas);
        ASSERT_EQ(domain->predicates.size(), expect.num_predicates);
        ASSERT_EQ(domain->constants.size(), expect.num_constants);
        ASSERT_EQ(domain->types.size(), expect.num_types);
    }

    void assert_problem_parse(const formalism::ProblemDescription& problem, const test::ProblemParseResult& expect)
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
            std::istringstream domain_stream(domain_blocks);
            std::istringstream problem_stream(problem_blocks);

            const auto domain = parsers::DomainParser::parse(domain_stream);
            const auto problem = parsers::ProblemParser::parse(domain, "", problem_stream);

            assert_domain_parse(domain, domain_blocks_parse_result);
            assert_problem_parse(problem, problem_blocks_parse_result);
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

        const auto domain = parsers::DomainParser::parse(domain_stream);
        const auto problem = parsers::ProblemParser::parse(domain, "", problem_stream);

        assert_domain_parse(domain, domain_result);
        assert_problem_parse(problem, problem_result);
    }

    INSTANTIATE_TEST_SUITE_P(ParamTest,
                             ParseTest,
                             testing::Values(std::make_tuple(domain_blocks, domain_blocks_parse_result, problem_blocks, problem_blocks_parse_result),
                                             std::make_tuple(domain_gripper, domain_gripper_parse_result, problem_gripper, problem_gripper_parse_result),
                                             std::make_tuple(domain_spider, domain_spider_parse_result, problem_spider, problem_spider_parse_result)));
}  // namespace test
