#ifndef MIMIR_TEST_INSTANCES_GRIPPER_DOMAIN_HPP_
#define MIMIR_TEST_INSTANCES_GRIPPER_DOMAIN_HPP_

#include "../../expect_structures.hpp"

#include <string>

namespace test::gripper
{
    static DomainParseResult domain_parse_result __attribute__((unused)) = { num_action_schemas : 3, num_predicates : 7, num_constants : 0, num_types : 1 };

    static std::string domain = R"(
(define (domain gripper-strips)
   (:predicates (room ?r)
		        (ball ?b)
		        (gripper ?g)
		        (at-robby ?r)
		        (at ?b ?r)
		        (free ?g)
		        (carry ?o ?g))

   (:action move
       :parameters (?from ?to)
       :precondition (and (room ?from)
                          (room ?to)
                          (at-robby ?from))
       :effect (and (at-robby ?to)
		            (not (at-robby ?from))))

   (:action pick
       :parameters (?obj ?room ?gripper)
       :precondition (and (ball ?obj)
                          (room ?room)
                          (gripper ?gripper)
			              (at ?obj ?room)
                          (at-robby ?room)
                          (free ?gripper))
       :effect (and (carry ?obj ?gripper)
		            (not (at ?obj ?room))
		            (not (free ?gripper))))

   (:action drop
       :parameters (?obj ?room ?gripper)
       :precondition (and (ball ?obj)
                          (room ?room)
                          (gripper ?gripper)
			              (carry ?obj ?gripper)
                          (at-robby ?room))
       :effect (and (at ?obj ?room)
		            (free ?gripper)
		            (not (carry ?obj ?gripper)))))
)";
}  // namespace test

#endif  // MIMIR_TEST_INSTANCES_GRIPPER_DOMAIN_HPP_
