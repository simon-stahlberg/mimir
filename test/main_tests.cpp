#include <gtest/gtest.h>

#include <span>

// #include "pybind11/embed.h"

int main(int argc, char** argv)
{
   auto cmd_args = std::span{argv, static_cast< size_t >(argc)};
   // now run all defined GTests
   ::testing::InitGoogleTest(&argc, argv);
   return RUN_ALL_TESTS();
}