#include "../include/mimir/formalism/domain.hpp"
#include "../include/mimir/formalism/problem.hpp"
#include "../include/mimir/generators/complete_state_space.hpp"
#include "../include/mimir/generators/goal_matcher.hpp"
#include "../include/mimir/generators/successor_generator_factory.hpp"
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
    TEST(Ground, Blocks)
    {
        std::istringstream domain_stream(blocks::domain);
        std::istringstream problem_stream(blocks::problem);

        const auto domain = mimir::parsers::DomainParser::parse(domain_stream);
        const auto problem = mimir::parsers::ProblemParser::parse(domain, "", problem_stream);
        const auto predicates_by_name = domain->get_predicate_name_map();

        const auto successor_generator = mimir::planners::create_sucessor_generator(problem, mimir::planners::GROUNDED);
        const auto state_space = mimir::planners::create_complete_state_space(problem, successor_generator);
        mimir::planners::GoalMatcher goal_matcher(state_space);

        const auto on_predicate = predicates_by_name.at("on");
        const auto [state, cost] = goal_matcher.best_match({ mimir::formalism::create_atom(on_predicate, on_predicate->parameters) });

        ASSERT_EQ(cost, 2);
    }

    TEST(Ground, Gripper)
    {
        std::istringstream domain_stream(gripper::domain);
        std::istringstream problem_stream(gripper::problem);

        const auto domain = mimir::parsers::DomainParser::parse(domain_stream);
        const auto problem = mimir::parsers::ProblemParser::parse(domain, "", problem_stream);
        const auto predicates_by_name = domain->get_predicate_name_map();

        const auto successor_generator = mimir::planners::create_sucessor_generator(problem, mimir::planners::GROUNDED);
        const auto state_space = mimir::planners::create_complete_state_space(problem, successor_generator);
        mimir::planners::GoalMatcher goal_matcher(state_space);

        const auto gripper_predicate = predicates_by_name.at("gripper");
        const auto ball_predicate = predicates_by_name.at("ball");

        const auto gripper_atom = mimir::formalism::create_atom(gripper_predicate, gripper_predicate->parameters);
        const auto ball_atom = mimir::formalism::create_atom(ball_predicate, ball_predicate->parameters);

        const auto [state, cost] = goal_matcher.best_match({ gripper_atom, ball_atom });

        ASSERT_EQ(cost, 0);
    }
}  // namespace test
