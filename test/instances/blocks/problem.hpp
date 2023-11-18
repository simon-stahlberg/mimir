#ifndef MIMIR_TEST_INSTANCES_BLOCKS_PROBLEM_HPP_
#define MIMIR_TEST_INSTANCES_BLOCKS_PROBLEM_HPP_

#include "../../expect_structures.hpp"

#include <string>

namespace test::blocks
{
    static ProblemParseResult problem_parse_result __attribute__((unused)) = { num_objects : 4, num_initial : 9, num_goal : 3 };

    static int32_t bfs_result[] = { 0, 1, 5, 17, 41, 77, 101 };

    static constexpr int32_t bfs_length = std::extent<decltype(bfs_result)>::value;

    static int32_t bfs_plan_length __attribute__((unused)) = 6;

    static std::string problem = R"(
(define (problem blocks-4-0)
    (:domain blocks)

    (:objects d b a c )

    (:init (clear c)
           (clear a)
           (clear b)
           (clear d)
           (ontable c)
           (ontable a)
           (ontable b)
           (ontable d)
           (handempty))

    (:goal (and (on d c)
                (on c b)
                (on b a)))
    )
)";
}  // namespace test
#endif  // MIMIR_TEST_INSTANCES_BLOCKS_PROBLEM_HPP_
