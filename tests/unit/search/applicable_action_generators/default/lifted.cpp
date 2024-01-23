#include <mimir/search/applicable_action_generators.hpp>

#include <gtest/gtest.h>


namespace mimir::tests
{

TEST(MimirTests, SearchApplicableActionGeneratorsDefaultLiftedTest) {
    // Instantiate lifted version
    auto lifted_successor_generator = DefaultApplicableActionGenerator<Lifted>();
}

}
