#include "../mimir/formalism/domain.hpp"
#include "../mimir/formalism/problem.hpp"
#include "../mimir/generators/state_space.hpp"
#include "../mimir/generators/successor_generator.hpp"
#include "../mimir/generators/successor_generator_factory.hpp"
#include "../mimir/pddl/parsers.hpp"

// PDDL files encoded as static strings named "domain_<DOMAIN>" or "problem_<DOMAIN>"
// The expected results are stored as structs named "domain_<DOMAIN>_<TYPE>_result" and "problem_<DOMAIN>_<TYPE>_result"
// Where <TYPE> is a type of result, e.g., parsing or expanding.

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
    class ExpandTest : public testing::TestWithParam<std::tuple<std::string, DomainParseResult, std::string, ProblemParseResult>>
    {
    };

    TEST_P(ExpandTest, Parameterized)
    {
        const auto domain_text = std::get<0>(GetParam());
        const auto domain_result = std::get<1>(GetParam());
        const auto problem_text = std::get<2>(GetParam());
        const auto problem_result = std::get<3>(GetParam());

        std::istringstream domain_stream(domain_text);
        std::istringstream problem_stream(problem_text);

        const auto domain = parsers::DomainParser::parse(domain_stream);
        const auto problem = parsers::ProblemParser::parse(domain, "", problem_stream);

        const auto successor_generator = planners::create_sucessor_generator(problem, planners::SuccessorGeneratorType::GROUNDED);
        const auto state_space = planners::create_state_space(problem, successor_generator);

        ASSERT_GT(state_space->num_states(), 0);
    }

    INSTANTIATE_TEST_SUITE_P(ParamTest,
                             ExpandTest,
                             testing::Values(std::make_tuple(domain_blocks, domain_blocks_parse_result, problem_blocks, problem_blocks_parse_result),
                                             std::make_tuple(domain_gripper, domain_gripper_parse_result, problem_gripper, problem_gripper_parse_result),
                                             std::make_tuple(domain_spanner, domain_spanner_parse_result, problem_spanner, problem_spanner_parse_result),
                                             std::make_tuple(domain_spider, domain_spider_parse_result, problem_spider, problem_spider_parse_result)));
}  // namespace test
