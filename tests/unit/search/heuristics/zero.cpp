#include <mimir/search/config.hpp>
#include <mimir/search/heuristics/zero.hpp>

#include <gtest/gtest.h>


namespace mimir::tests
{

TEST(MimirTests, SearchHeuristicsZeroTest) {
    auto problem = Problem(nullptr);

    // Instantiate ground version
    auto zero_heuristic_grounded = Heuristic<ZeroHeuristicTag<GroundedTag>>(problem);

    // Instantiate lifted version
    auto zero_heuristic_lifted = Heuristic<ZeroHeuristicTag<LiftedTag>>(problem);
}



}
