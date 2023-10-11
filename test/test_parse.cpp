#include "../mimir/formalism/domain.hpp"
#include "../mimir/formalism/problem.hpp"
#include "../mimir/pddl/parsers.hpp"

// PDDL files encoded as static strings named "domain_<DOMAIN>" or "problem_<DOMAIN>"
// The expected results are stored as structs named "domain_<DOMAIN>_result" and "problem_<DOMAIN>_result"
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

    TEST(PDDLTest, ParseFiles)
    {
        std::istringstream domain_stream(domain_gripper);
        std::istringstream problem_stream(problem_gripper);

        const auto domain = parsers::DomainParser::parse(domain_stream);
        const auto problem = parsers::ProblemParser::parse(domain, "", problem_stream);

        assert_domain_parse(domain, domain_gripper_result);
        assert_problem_parse(problem, problem_gripper_result);
    }
}  // namespace test
