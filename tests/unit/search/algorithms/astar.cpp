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
    auto grounded_astar = Algorithm<AStar<Grounded, Heuristic<Zero<Grounded>>>>(problem);
    // different syntax:
    // auto ... = Algorithm<AStarTag<GroundedTag, Heuristic<ZeroTag<GroundedTag>>>>(problem);
    // is a little bit more expressive than
    // auto ... = AStar<GroundedTag, ZeroHeuristic<GroundedTag>>>>(problem);
    // But also allows providing new alternatives easily:
    // auto ... = AStar2<GroundedTag, ZeroHeuristic<GroundedTag>>>>(problem);

    // auto grounded_astar = Algorithm<AStar2Tag<GroundedTag, Heuristic<ZeroTag<GroundedTag>>>>(problem);
    GroundActionList plan;
    const auto search_status = grounded_astar.find_solution(plan);
}


TEST(MimirTests, LiftedAstarTest) {
    // Instantiate lifted version
    auto problem = static_cast<Problem>(nullptr);
    auto lifted_astar = Algorithm<AStar<Lifted, Heuristic<Zero<Lifted>>>>(problem);
    GroundActionList plan;
    const auto search_status = lifted_astar.find_solution(plan);
}

}
