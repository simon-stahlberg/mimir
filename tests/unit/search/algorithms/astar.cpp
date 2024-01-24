#include <mimir/formalism/problem/declarations.hpp>

#include <mimir/search/algorithms.hpp>
#include <mimir/search/heuristics.hpp>

#include <gtest/gtest.h>


namespace mimir::tests
{

TEST(MimirTests, SearchAlgorithmsAstarGroundedBlindTest) {
    // Instantiate grounded version
    auto problem = static_cast<Problem>(nullptr);
    auto grounded_astar = Algorithm<AStarTag<GroundedTag, BlindTag>>(problem);
    using ActionViewList = typename TypeTraits<decltype(grounded_astar)>::ActionViewList;
    ActionViewList plan;
    const auto search_status = grounded_astar.find_solution(plan);
}


TEST(MimirTests, SearchAlgorithmsAstarLiftedBlindTest) {
    // Instantiate lifted version
    auto problem = static_cast<Problem>(nullptr);
    auto lifted_astar = Algorithm<AStarTag<LiftedTag, BlindTag>>(problem);
    using ActionViewList = typename TypeTraits<decltype(lifted_astar)>::ActionViewList;
    ActionViewList plan;
    const auto search_status = lifted_astar.find_solution(plan);
}

}
