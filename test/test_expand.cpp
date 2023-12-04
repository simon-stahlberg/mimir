#include "../include/mimir/formalism/domain.hpp"
#include "../include/mimir/formalism/problem.hpp"
#include "../include/mimir/generators/complete_state_space.hpp"
#include "../include/mimir/generators/successor_generator.hpp"
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
    mimir::formalism::AtomSet to_set(const mimir::formalism::AtomList& list)
    {
        mimir::formalism::AtomSet set;

        for (const auto& item : list)
        {
            set.emplace(item);
        }

        return set;
    }

    class ExpandTest : public testing::TestWithParam<std::tuple<std::string, DomainParseResult, std::string, ProblemParseResult>>
    {
    };

    TEST_P(ExpandTest, Parameterized)
    {
        const auto domain_text = std::get<0>(GetParam());
        // const auto domain_result = std::get<1>(GetParam());
        const auto problem_text = std::get<2>(GetParam());
        // const auto problem_result = std::get<3>(GetParam());

        std::istringstream domain_stream(domain_text);
        std::istringstream problem_stream(problem_text);

        const auto domain = mimir::parsers::DomainParser::parse(domain_stream);
        const auto problem = mimir::parsers::ProblemParser::parse(domain, "", problem_stream);

        const auto successor_generator = mimir::planners::create_sucessor_generator(problem, mimir::planners::SuccessorGeneratorType::GROUNDED);
        const auto state_space = mimir::planners::create_complete_state_space(problem, successor_generator);

        const auto& states = state_space->get_states();
        ASSERT_GT(states.size(), 0);
        ASSERT_EQ(states.size(), state_space->num_states());
        std::equal_to<mimir::formalism::State> state_equals;

        for (const auto& state : states)
        {
            const auto dynamic_atom_list = state->get_dynamic_atoms();
            const auto static_atom_list = state->get_static_atoms();
            const auto all_atom_list = state->get_atoms();

            ASSERT_EQ(dynamic_atom_list.size() + static_atom_list.size(), all_atom_list.size());

            const auto dynamic_atom_set = to_set(dynamic_atom_list);
            const auto static_atom_set = to_set(static_atom_list);
            const auto all_atom_set = to_set(all_atom_list);

            ASSERT_EQ(static_atom_set, problem->get_static_atoms());
            ASSERT_EQ(dynamic_atom_list.size(), dynamic_atom_set.size());
            ASSERT_EQ(static_atom_list.size(), static_atom_set.size());
            ASSERT_EQ(all_atom_list.size(), all_atom_set.size());

            mimir::formalism::AtomSet new_all_atom_set;
            std::set_union(dynamic_atom_set.begin(),
                           dynamic_atom_set.end(),
                           static_atom_set.begin(),
                           static_atom_set.end(),
                           std::inserter(new_all_atom_set, new_all_atom_set.begin()));

            ASSERT_EQ(new_all_atom_set.size(), all_atom_set.size());

            const auto new_state = mimir::formalism::create_state(new_all_atom_set, problem);
            ASSERT_TRUE(state_equals(new_state, state));
        }
    }

    INSTANTIATE_TEST_SUITE_P(ParamTest,
                             ExpandTest,
                             testing::Values(std::make_tuple(blocks::domain, blocks::domain_parse_result, blocks::problem, blocks::problem_parse_result),
                                             std::make_tuple(gripper::domain, gripper::domain_parse_result, gripper::problem, gripper::problem_parse_result),
                                             std::make_tuple(spanner::domain, spanner::domain_parse_result, spanner::problem, spanner::problem_parse_result),
                                             std::make_tuple(spider::domain, spider::domain_parse_result, spider::problem, spider::problem_parse_result)));
}  // namespace test
