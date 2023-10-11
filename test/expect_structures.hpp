#if !defined(TEST_EXPECT_STRUCTURES_HPP_)
#define TEST_EXPECT_STRUCTURES_HPP_

#include <cstddef>
namespace test
{
    struct DomainParseResult
    {
        std::size_t num_action_schemas;
        std::size_t num_predicates;
        std::size_t num_constants;
        std::size_t num_types;
    };

    struct ProblemParseResult
    {
        std::size_t num_objects;
        std::size_t num_initial;
        std::size_t num_goal;
    };
}  // namespace test
#endif  // TEST_EXPECT_STRUCTURES_HPP_
