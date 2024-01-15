#include <mimir/common/config.hpp>
#include <mimir/search/successor_generator.hpp>

#include <gtest/gtest.h>


namespace mimir::tests
{

TEST(MimirTests, GroundedSuccessorGeneratorTest) {
    // Instantiate grounded version
    auto grounded_successor_generator = search::SuccessorGenerator<Grounded>();
} 


TEST(MimirTests, LiftedSuccessorGeneratorTest) {
    // Instantiate lifted version 
    auto lifted_successor_generator = search::SuccessorGenerator<Lifted>();
} 

} 
