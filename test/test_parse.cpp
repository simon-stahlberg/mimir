#include "../mimir/formalism/domain.hpp"
#include "../mimir/formalism/problem.hpp"
#include "../mimir/pddl/parsers.hpp"

// PDDL files encoded as static strings named "domain_<DOMAIN>" or "problem_<DOMAIN>"
// The expected results are stored as structs named "domain_<DOMAIN>_result" and "problem_<DOMAIN>_result"
#include "instances/blocks/domain.hpp"
#include "instances/blocks/problem.hpp"
#include "instances/gripper/domain.hpp"
#include "instances/gripper/problem.hpp"

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

    class PDDLTest : public testing::TestWithParam<std::tuple<std::string, DomainParseResult, std::string, ProblemParseResult>>
    {
    };

    TEST(PDDLTest, ParseMultipleTimes)
    {
        std::istringstream domain_stream_1(domain_blocks);
        std::istringstream problem_stream_1(problem_blocks);

        const auto domain_1 = parsers::DomainParser::parse(domain_stream_1);
        const auto problem_1 = parsers::ProblemParser::parse(domain_1, "", problem_stream_1);

        std::istringstream domain_stream_2(domain_blocks);
        std::istringstream problem_stream_2(problem_blocks);

        const auto domain_2 = parsers::DomainParser::parse(domain_stream_2);
        const auto problem_2 = parsers::ProblemParser::parse(domain_2, "", problem_stream_2);

        assert_domain_parse(domain_1, domain_blocks_result);
        assert_domain_parse(domain_2, domain_blocks_result);
        assert_problem_parse(problem_1, problem_blocks_result);
        assert_problem_parse(problem_2, problem_blocks_result);
    }

    TEST_P(PDDLTest, ParseFiles)
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

    INSTANTIATE_TEST_SUITE_P(PDDLFiles,
                             PDDLTest,
                             testing::Values(std::make_tuple(domain_blocks, domain_blocks_result, problem_blocks, problem_blocks_result),
                                             std::make_tuple(domain_gripper, domain_gripper_result, problem_gripper, problem_gripper_result)));
}  // namespace test
