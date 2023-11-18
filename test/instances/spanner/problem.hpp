#ifndef MIMIR_TEST_INSTANCES_SPANNER_PROBLEM_HPP_
#define MIMIR_TEST_INSTANCES_SPANNER_PROBLEM_HPP_

#include "../../expect_structures.hpp"

#include <string>

namespace test::spanner
{
    static ProblemParseResult problem_parse_result __attribute__((unused)) = { num_objects : 8, num_initial : 9, num_goal : 1 };

    static int32_t bfs_result[] = { 0, 1, 2, 3, 4, 6, 7 };

    static constexpr int32_t bfs_length = std::extent<decltype(bfs_result)>::value;

    static int32_t bfs_plan_length __attribute__((unused)) = 6;

    static std::string problem = R"(
(define (problem prob)
    (:domain spanner)

    (:objects bob - man
              spanner1 - spanner
              nut1 - nut
              location1 location2 location3 - location
              shed gate - location)

    (:init (at bob shed)
           (at spanner1 location3)
           (useable spanner1)
           (loose nut1)
           (at nut1 gate)
           (link shed location1)
           (link location3 gate)
           (link location1 location2)
           (link location2 location3))

    (:goal (and (tightened nut1))))
)";
}  // namespace test

#endif  // MIMIR_TEST_INSTANCES_SPANNER_PROBLEM_HPP_
