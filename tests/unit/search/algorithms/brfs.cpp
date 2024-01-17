#include <mimir/formalism/problem.hpp>

#include <mimir/search/config.hpp>
#include <mimir/search/algorithms/brfs.hpp>

#include <gtest/gtest.h>


namespace mimir::tests
{

TEST(MimirTests, GroundedBrFSTest) {
    // Instantiate grounded version
    auto problem = static_cast<Problem>(nullptr);
    auto grounded_brfs = BrFS<Grounded>(problem);
    grounded_brfs.find_solution();
}


TEST(MimirTests, LiftedBrFSTest) {
    // Instantiate lifted version
    auto problem = static_cast<Problem>(nullptr);
    auto lifted_brfs = BrFS<Lifted>(problem);
    lifted_brfs.find_solution();
}

}
