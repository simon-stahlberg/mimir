#ifndef MIMIR_TEST_INSTANCES_SPIDER_PROBLEM_HPP_
#define MIMIR_TEST_INSTANCES_SPIDER_PROBLEM_HPP_

#include "../../expect_structures.hpp"

#include <string>

namespace test::spider
{
    static ProblemParseResult problem_parse_result __attribute__((unused)) = { num_objects : 19, num_initial : 92, num_goal : 17 };

    static int32_t bfs_result[] = { 0,    1,    4,    7,    10,   13,   22,   29,   36,   41,   56,    67,    76,    93,    115,   128,   154,
                                    183,  213,  243,  288,  340,  397,  453,  523,  615,  715,  798,   916,   1018,  1143,  1261,  1414,  1543,
                                    1720, 1900, 2065, 2261, 2441, 2649, 2872, 3098, 3348, 3612, 3897,  4186,  4507,  4836,  5204,  5549,  5993,
                                    6350, 6756, 7118, 7525, 7875, 8267, 8579, 8945, 9308, 9707, 10043, 10437, 10814, 11222, 11612, 12035, 12452 };

    static constexpr int32_t bfs_length = std::extent<decltype(bfs_result)>::value;

    static int32_t bfs_plan_length __attribute__((unused)) = 67;

    static std::string problem = R"(
(define (problem spider-1-4-3-3-2-1400)
    (:domain spider)

    (:objects card-d0-s0-v0 - card
              card-d0-s0-v1 - card
              card-d0-s0-v2 - card
              card-d0-s1-v0 - card
              card-d0-s1-v1 - card
              card-d0-s1-v2 - card
              card-d0-s2-v0 - card
              card-d0-s2-v1 - card
              card-d0-s2-v2 - card
              card-d0-s3-v0 - card
              card-d0-s3-v1 - card
              card-d0-s3-v2 - card
              pile-0 - tableau
              pile-1 - tableau
              pile-2 - tableau
              deal-0 - deal
              deal-1 - deal
              deal-2 - deal)

    (:init (on card-d0-s3-v2 card-d0-s1-v2)
           (on card-d0-s1-v2 pile-0)
           (clear card-d0-s3-v2)
           (part-of-tableau pile-0 pile-0)
           (part-of-tableau card-d0-s1-v2 pile-0)
           (part-of-tableau card-d0-s3-v2 pile-0)
           (movable card-d0-s3-v2)
           (in-play card-d0-s1-v2)
           (in-play card-d0-s3-v2)
           (on card-d0-s2-v2 card-d0-s2-v1)
           (on card-d0-s2-v1 pile-1)
           (clear card-d0-s2-v2)
           (part-of-tableau pile-1 pile-1)
           (part-of-tableau card-d0-s2-v1 pile-1)
           (part-of-tableau card-d0-s2-v2 pile-1)
           (movable card-d0-s2-v2)
           (in-play card-d0-s2-v1)
           (in-play card-d0-s2-v2)
           (on card-d0-s0-v1 card-d0-s2-v0)
           (on card-d0-s2-v0 pile-2)
           (clear card-d0-s0-v1)
           (part-of-tableau pile-2 pile-2)
           (part-of-tableau card-d0-s2-v0 pile-2)
           (part-of-tableau card-d0-s0-v1 pile-2)
           (movable card-d0-s0-v1)
           (in-play card-d0-s2-v0)
           (in-play card-d0-s0-v1)
           (on card-d0-s3-v1 deal-0)
           (on card-d0-s0-v0 card-d0-s3-v1)
           (on card-d0-s1-v0 card-d0-s0-v0)
           (clear card-d0-s1-v0)
           (on card-d0-s3-v0 deal-1)
           (on card-d0-s0-v2 card-d0-s3-v0)
           (on card-d0-s1-v1 card-d0-s0-v2)
           (clear card-d0-s1-v1)
           (current-deal deal-0)
           (can-continue-group card-d0-s0-v0 card-d0-s0-v1)
           (can-continue-group card-d0-s0-v1 card-d0-s0-v2)
           (can-continue-group card-d0-s1-v0 card-d0-s1-v1)
           (can-continue-group card-d0-s1-v1 card-d0-s1-v2)
           (can-continue-group card-d0-s2-v0 card-d0-s2-v1)
           (can-continue-group card-d0-s2-v1 card-d0-s2-v2)
           (can-continue-group card-d0-s3-v0 card-d0-s3-v1)
           (can-continue-group card-d0-s3-v1 card-d0-s3-v2)
           (can-be-placed-on card-d0-s0-v0 card-d0-s0-v1)
           (can-be-placed-on card-d0-s0-v1 card-d0-s0-v2)
           (can-be-placed-on card-d0-s0-v0 card-d0-s1-v1)
           (can-be-placed-on card-d0-s0-v1 card-d0-s1-v2)
           (can-be-placed-on card-d0-s0-v0 card-d0-s2-v1)
           (can-be-placed-on card-d0-s0-v1 card-d0-s2-v2)
           (can-be-placed-on card-d0-s0-v0 card-d0-s3-v1)
           (can-be-placed-on card-d0-s0-v1 card-d0-s3-v2)
           (can-be-placed-on card-d0-s1-v0 card-d0-s0-v1)
           (can-be-placed-on card-d0-s1-v1 card-d0-s0-v2)
           (can-be-placed-on card-d0-s1-v0 card-d0-s1-v1)
           (can-be-placed-on card-d0-s1-v1 card-d0-s1-v2)
           (can-be-placed-on card-d0-s1-v0 card-d0-s2-v1)
           (can-be-placed-on card-d0-s1-v1 card-d0-s2-v2)
           (can-be-placed-on card-d0-s1-v0 card-d0-s3-v1)
           (can-be-placed-on card-d0-s1-v1 card-d0-s3-v2)
           (can-be-placed-on card-d0-s2-v0 card-d0-s0-v1)
           (can-be-placed-on card-d0-s2-v1 card-d0-s0-v2)
           (can-be-placed-on card-d0-s2-v0 card-d0-s1-v1)
           (can-be-placed-on card-d0-s2-v1 card-d0-s1-v2)
           (can-be-placed-on card-d0-s2-v0 card-d0-s2-v1)
           (can-be-placed-on card-d0-s2-v1 card-d0-s2-v2)
           (can-be-placed-on card-d0-s2-v0 card-d0-s3-v1)
           (can-be-placed-on card-d0-s2-v1 card-d0-s3-v2)
           (can-be-placed-on card-d0-s3-v0 card-d0-s0-v1)
           (can-be-placed-on card-d0-s3-v1 card-d0-s0-v2)
           (can-be-placed-on card-d0-s3-v0 card-d0-s1-v1)
           (can-be-placed-on card-d0-s3-v1 card-d0-s1-v2)
           (can-be-placed-on card-d0-s3-v0 card-d0-s2-v1)
           (can-be-placed-on card-d0-s3-v1 card-d0-s2-v2)
           (can-be-placed-on card-d0-s3-v0 card-d0-s3-v1)
           (can-be-placed-on card-d0-s3-v1 card-d0-s3-v2)
           (is-ace card-d0-s0-v0)
           (is-ace card-d0-s1-v0)
           (is-ace card-d0-s2-v0)
           (is-ace card-d0-s3-v0)
           (is-king card-d0-s0-v2)
           (is-king card-d0-s1-v2)
           (is-king card-d0-s2-v2)
           (is-king card-d0-s3-v2)
           (next-deal deal-0 deal-1)
           (next-deal deal-1 deal-2)
           (to-deal card-d0-s3-v1 pile-2 deal-0 deal-0)
           (to-deal card-d0-s1-v0 pile-0 deal-0 card-d0-s0-v0)
           (to-deal card-d0-s0-v0 pile-1 deal-0 card-d0-s3-v1)
           (to-deal card-d0-s3-v0 pile-2 deal-1 deal-1)
           (to-deal card-d0-s1-v1 pile-0 deal-1 card-d0-s0-v2)
           (to-deal card-d0-s0-v2 pile-1 deal-1 card-d0-s3-v0)
           (= (total-cost) 0))

    (:goal (and (clear pile-0)
                (clear pile-1)
                (clear pile-2)
                (clear deal-0)
                (clear deal-1)
                (on card-d0-s0-v0 discard)
                (on card-d0-s0-v1 discard)
                (on card-d0-s0-v2 discard)
                (on card-d0-s1-v0 discard)
                (on card-d0-s1-v1 discard)
                (on card-d0-s1-v2 discard)
                (on card-d0-s2-v0 discard)
                (on card-d0-s2-v1 discard)
                (on card-d0-s2-v2 discard)
                (on card-d0-s3-v0 discard)
                (on card-d0-s3-v1 discard)
                (on card-d0-s3-v2 discard)))

    (:metric minimize (total-cost)))
)";
}  // namespace test

#endif  // MIMIR_TEST_INSTANCES_SPIDER_PROBLEM_HPP_
