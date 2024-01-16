#include <mimir/search/config.hpp>
#include <mimir/search/algorithms/astar.hpp>
#include <mimir/search/heuristics/zero.hpp>

#include <gtest/gtest.h>


namespace mimir::tests
{

TEST(MimirTests, GroundedAstarTest) {
    // Instantiate grounded version
    auto grounded_astar = AStar<Grounded, ZeroHeuristic>();
    grounded_astar.find_solution();
}


TEST(MimirTests, LiftedAstarTest) {
    // Instantiate lifted version
    auto lifted_astar = AStar<Lifted, ZeroHeuristic>();
    lifted_astar.find_solution();
}

}
