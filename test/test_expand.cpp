#include "../mimir/formalism/domain.hpp"
#include "../mimir/formalism/problem.hpp"
#include "../mimir/generators/state_space.hpp"
#include "../mimir/generators/successor_generator.hpp"
#include "../mimir/generators/successor_generator_factory.hpp"
#include "../mimir/pddl/parsers.hpp"

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
                             testing::Values(std::make_tuple(blocks::domain, blocks::domain_parse_result, blocks::problem, blocks::problem_parse_result),
                                             std::make_tuple(gripper::domain, gripper::domain_parse_result, gripper::problem, gripper::problem_parse_result),
                                             std::make_tuple(spanner::domain, spanner::domain_parse_result, spanner::problem, spanner::problem_parse_result),
                                             std::make_tuple(spider::domain, spider::domain_parse_result, spider::problem, spider::problem_parse_result)));
}  // namespace test
