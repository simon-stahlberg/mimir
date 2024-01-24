#include <mimir/formalism/problem/declarations.hpp>

#include <mimir/search/successor_state_generators.hpp>

#include <gtest/gtest.h>


namespace mimir::tests
{

TEST(MimirTests, SearchSSGDefaultLiftedTest) {
    // Instantiate lifted version
    auto lifted_state_repository = DefaultSSG<LiftedTag>();
    auto problem = static_cast<Problem>(nullptr);
    const auto initial_state = lifted_state_repository.get_or_create_initial_state(problem);
}

}
