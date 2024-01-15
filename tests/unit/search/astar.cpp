#include <mimir/common/config.hpp>
#include <mimir/search/astar.hpp>

#include <gtest/gtest.h>


namespace mimir::tests
{

TEST(MimirTests, GroundedAstarTest) {
    // Instantiate grounded version
    auto grounded_astar = search::AStar<Grounded>();
} 


TEST(MimirTests, LiftedAstarTest) {
    // Instantiate lifted version 
    auto lifted_astar = search::AStar<Lifted>();
} 

} 
