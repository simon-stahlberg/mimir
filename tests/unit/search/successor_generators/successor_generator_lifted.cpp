#include <mimir/common/config.hpp>
#include <mimir/search/successor_generators/successor_generator_lifted.hpp>

#include <gtest/gtest.h>


namespace mimir::tests
{

TEST(MimirTests, LiftedSuccessorGeneratorTest) {
    // Instantiate lifted version
    auto lifted_successor_generator = SuccessorGenerator<Lifted>();
}

}
