#include <mimir/search/heuristics.hpp>

#include <gtest/gtest.h>


namespace mimir::tests
{

TEST(MimirTests, SearchHeuristicsZeroTest) {
    auto problem = Problem(nullptr);

    // Instantiate ground version
    auto zero_heuristic_grounded = Heuristic<HeuristicDispatcher<BlindTag, GroundedTag, BitsetStateTag>>(problem);

    // Instantiate lifted version
    auto zero_heuristic_lifted = Heuristic<HeuristicDispatcher<BlindTag, LiftedTag, BitsetStateTag>>(problem);
}



}
