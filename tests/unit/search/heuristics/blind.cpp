#include "mimir/search/heuristics.hpp"

#include <gtest/gtest.h>

namespace mimir::tests
{

TEST(MimirTests, SearchHeuristicsZeroTest)
{
    auto problem = Problem(nullptr);

    auto zero_heuristic = Heuristic<HeuristicDispatcher<BlindTag, DenseStateTag>>();
}

}
