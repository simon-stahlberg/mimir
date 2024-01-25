#include <mimir/formalism/problem/declarations.hpp>

#include <mimir/search/successor_state_generators.hpp>

#include <gtest/gtest.h>


namespace mimir::tests
{

TEST(MimirTests, SearchSSGDefaultLiftedTest) {
    // Instantiate lifted version
    auto lifted_ssg = SSG<SSGDispatcher<DefaultSSGTag, LiftedTag, BitsetStateTag, DefaultActionTag>>();
    auto problem = static_cast<Problem>(nullptr);
    const auto initial_state = lifted_ssg.get_or_create_initial_state(problem);
}

}
