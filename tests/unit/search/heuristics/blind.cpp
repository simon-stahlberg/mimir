#include <mimir/search/heuristics.hpp>

#include <gtest/gtest.h>


namespace mimir::tests
{

TEST(MimirTests, SearchHeuristicsZeroTest) {
    auto problem = Problem(nullptr);

    // Instantiate ground version
    auto zero_heuristic_grounded = Heuristic<WrappedHeuristicTag<BlindTag, GroundedTag, BitsetStateTag, DefaultActionTag>>(problem);

    // Instantiate lifted version
    auto zero_heuristic_lifted = Heuristic<WrappedHeuristicTag<BlindTag, LiftedTag, BitsetStateTag, DefaultActionTag>>(problem);
}



}
