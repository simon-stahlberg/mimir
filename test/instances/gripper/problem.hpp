#ifndef MIMIR_TEST_INSTANCES_GRIPPER_PROBLEM_HPP_
#define MIMIR_TEST_INSTANCES_GRIPPER_PROBLEM_HPP_

#include "../../expect_structures.hpp"

#include <string>

namespace test::gripper
{
    static ProblemParseResult problem_parse_result __attribute__((unused)) = { num_objects : 8, num_initial : 15, num_goal : 4 };

    static int32_t bfs_result[] = { 0, 1, 10, 30, 46, 74, 104, 134, 182, 218, 234, 246 };

    static constexpr int32_t bfs_length = std::extent<decltype(bfs_result)>::value;

    static int32_t bfs_plan_length __attribute__((unused)) = 11;

    static std::string problem = R"(
(define (problem strips-gripper-x-1)
   (:domain gripper-strips)

   (:objects rooma roomb ball4 ball3 ball2 ball1 left right)

   (:init (room rooma)
          (room roomb)
          (ball ball4)
          (ball ball3)
          (ball ball2)
          (ball ball1)
          (at-robby rooma)
          (free left)
          (free right)
          (at ball4 rooma)
          (at ball3 rooma)
          (at ball2 rooma)
          (at ball1 rooma)
          (gripper left)
          (gripper right))

   (:goal (and (at ball4 roomb)
               (at ball3 roomb)
               (at ball2 roomb)
               (at ball1 roomb))))
)";
}  // namespace test

#endif  // MIMIR_TEST_INSTANCES_GRIPPER_PROBLEM_HPP_
