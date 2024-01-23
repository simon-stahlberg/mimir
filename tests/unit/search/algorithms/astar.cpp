#include <mimir/formalism/problem/declarations.hpp>

#include <mimir/search/algorithms.hpp>
#include <mimir/search/heuristics.hpp>

#include <gtest/gtest.h>


namespace mimir::tests
{

TEST(MimirTests, GroundedAstarTest) {
    // Instantiate grounded version
    auto problem = static_cast<Problem>(nullptr);
    auto grounded_astar = Algorithm<AStarTag<GroundedTag, BlindTag, DefaultApplicableActionGeneratorTag>>(problem);
    GroundActionList plan;
    const auto search_status = grounded_astar.find_solution(plan);
}


TEST(MimirTests, LiftedAstarTest) {
    // Instantiate lifted version
    auto problem = static_cast<Problem>(nullptr);
    auto lifted_astar = Algorithm<AStarTag<LiftedTag, BlindTag>>(problem);
    GroundActionList plan;
    const auto search_status = lifted_astar.find_solution(plan);
}

}
