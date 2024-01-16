#include <mimir/common/config.hpp>
#include <mimir/search/algorithms/brfs.hpp>

#include <gtest/gtest.h>


namespace mimir::tests
{

TEST(MimirTests, GroundedBrFSTest) {
    // Instantiate grounded version
    auto grounded_brfs = BreadthFirstSearch<Grounded>();
}


TEST(MimirTests, LiftedBrFSTest) {
    // Instantiate lifted version
    auto lifted_brfs = BreadthFirstSearch<Lifted>();
}

}
