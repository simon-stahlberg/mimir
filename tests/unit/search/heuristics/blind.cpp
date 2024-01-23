#include <mimir/search/config.hpp>
#include <mimir/search/heuristics/blind.hpp>

#include <gtest/gtest.h>


namespace mimir::tests
{

TEST(MimirTests, SearchHeuristicsZeroTest) {
    auto problem = Problem(nullptr);

    // Instantiate ground version
    auto zero_heuristic_grounded = Heuristic<Blind<Grounded>>(problem);

    // Instantiate lifted version
    auto zero_heuristic_lifted = Heuristic<Blind<Lifted>>(problem);
}



}
