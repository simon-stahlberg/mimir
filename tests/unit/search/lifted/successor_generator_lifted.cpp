#include <mimir/search/config.hpp>
#include <mimir/search/lifted/successor_generator.hpp>

#include <gtest/gtest.h>


namespace mimir::tests
{

TEST(MimirTests, LiftedSuccessorGeneratorTest) {
    // Instantiate lifted version
    auto lifted_successor_generator = SuccessorGenerator<Lifted>();
}

}
