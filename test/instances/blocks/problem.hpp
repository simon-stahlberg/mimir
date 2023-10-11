#if !defined(TEST_INSTANCES_BLOCKS_PROBLEM_HPP_)
#define TEST_INSTANCES_BLOCKS_PROBLEM_HPP_

#include "../../expect_structures.hpp"

#include <string>

namespace test
{
    static ProblemParseResult problem_blocks_result = { num_objects : 4, num_initial : 9, num_goal : 3 };

    static std::string problem_blocks = R"(
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
#endif  // TEST_INSTANCES_BLOCKS_PROBLEM_HPP_
