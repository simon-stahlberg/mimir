#if !defined(TEST_INSTANCES_SPANNER_PROBLEM_HPP_)
#define TEST_INSTANCES_SPANNER_PROBLEM_HPP_

#include "../../expect_structures.hpp"

#include <string>

namespace test
{
    static ProblemParseResult problem_spanner_parse_result = { num_objects : 8, num_initial : 9, num_goal : 1 };

    static std::string problem_spanner = R"(
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

#endif  // TEST_INSTANCES_SPANNER_PROBLEM_HPP_
