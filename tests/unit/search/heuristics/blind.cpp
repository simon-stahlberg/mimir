#include <mimir/search/heuristics.hpp>

#include <gtest/gtest.h>


namespace mimir::tests
{

TEST(MimirTests, SearchHeuristicsZeroTest) {
    auto problem = Problem(nullptr);

    // Instantiate ground version
    auto zero_heuristic_grounded = Heuristic<HeuristicInstantiation<BlindTag, GroundedTag>, GroundedTag>(problem);

    // Instantiate lifted version
    auto zero_heuristic_lifted = Heuristic<HeuristicInstantiation<BlindTag, LiftedTag>, LiftedTag>(problem);
}



}
