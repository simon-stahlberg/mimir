#include <mimir/search/config.hpp>
#include <mimir/search/lifted/applicable_action_generator.hpp>

#include <gtest/gtest.h>


namespace mimir::tests
{

TEST(MimirTests, LiftedApplicableActionGeneratorTest) {
    // Instantiate lifted version
    auto lifted_successor_generator = ApplicableActionGenerator<Lifted>();
}

}
