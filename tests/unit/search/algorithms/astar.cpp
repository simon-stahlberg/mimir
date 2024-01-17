#include <mimir/formalism/problem/declarations.hpp>

#include <mimir/search/config.hpp>
#include <mimir/search/algorithms/astar.hpp>
#include <mimir/search/heuristics/zero.hpp>

#include <gtest/gtest.h>


namespace mimir::tests
{

TEST(MimirTests, GroundedAstarTest) {
    // Instantiate grounded version
    auto problem = static_cast<Problem>(nullptr);
    auto grounded_astar = AStar<Grounded, ZeroHeuristic>(problem);
    grounded_astar.find_solution();
}


TEST(MimirTests, LiftedAstarTest) {
    // Instantiate lifted version
    auto problem = static_cast<Problem>(nullptr);
    auto lifted_astar = AStar<Lifted, ZeroHeuristic>(problem);
    lifted_astar.find_solution();
}

}
