#include <mimir/formalism/problem/declarations.hpp>

#include <mimir/search/algorithms.hpp>
#include <mimir/search/heuristics.hpp>

#include <gtest/gtest.h>


namespace mimir::tests
{

TEST(MimirTests, GroundedBrFSTest) {
    // Instantiate grounded version
    auto problem = static_cast<Problem>(nullptr);
    auto grounded_brfs = Algorithm<BrFSTag<GroundedTag>>(problem);
    using ActionViewList = typename TypeTraits<decltype(grounded_brfs)>::ActionViewList;
    ActionViewList plan;
    const auto search_status = grounded_brfs.find_solution(plan);

}


TEST(MimirTests, LiftedBrFSTest) {
    // Instantiate lifted version
    auto problem = static_cast<Problem>(nullptr);
    auto lifted_brfs = Algorithm<BrFSTag<GroundedTag>>(problem);
    using ActionViewList = typename TypeTraits<decltype(lifted_brfs)>::ActionViewList;
    ActionViewList plan;
    const auto search_status = lifted_brfs.find_solution(plan);
}

}
