#include <mimir/formalism/problem/declarations.hpp>

#include <mimir/search/config.hpp>
#include <mimir/search/algorithms/brfs.hpp>

#include <gtest/gtest.h>


namespace mimir::tests
{

TEST(MimirTests, GroundedBrFSTest) {
    // Instantiate grounded version
    auto problem = static_cast<Problem>(nullptr);
    auto grounded_brfs = BrFS<Grounded>(problem);
    GroundActionList plan;
    const auto search_status = grounded_brfs.find_solution(plan);

}


TEST(MimirTests, LiftedBrFSTest) {
    // Instantiate lifted version
    auto problem = static_cast<Problem>(nullptr);
    auto lifted_brfs = BrFS<Lifted>(problem);
    GroundActionList plan;
    const auto search_status = lifted_brfs.find_solution(plan);
}

}
