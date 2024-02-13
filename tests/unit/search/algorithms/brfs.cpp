#include <mimir/formalism/problem/declarations.hpp>

#include <mimir/search/algorithms.hpp>
#include <mimir/search/heuristics.hpp>

#include <gtest/gtest.h>


namespace mimir::tests
{

TEST(MimirTests, SearchAlgorithmsBrFSGroundedTest) {
    // Instantiate grounded version
    auto problem = static_cast<Problem>(nullptr);
    auto grounded_brfs = Algorithm<AlgorithmDispatcher<BrFSTag<GroundedTag>>>(problem);
    using ConstActionViewList = typename TypeTraits<decltype(grounded_brfs)>::ConstActionViewList;
    ConstActionViewList plan;
    const auto search_status = grounded_brfs.find_solution(plan);

}


TEST(MimirTests, SearchAlgorithmsBrFSLiftedTest) {
    // Instantiate lifted version
    auto problem = static_cast<Problem>(nullptr);
    auto lifted_brfs = Algorithm<AlgorithmDispatcher<BrFSTag<LiftedTag>>>(problem);
    using ConstActionViewList = typename TypeTraits<decltype(lifted_brfs)>::ConstActionViewList;
    ConstActionViewList plan;
    const auto search_status = lifted_brfs.find_solution(plan);
}

}
