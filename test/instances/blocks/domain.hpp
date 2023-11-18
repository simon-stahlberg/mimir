#ifndef MIMIR_TEST_INSTANCES_BLOCKS_DOMAIN_HPP_
#define MIMIR_TEST_INSTANCES_BLOCKS_DOMAIN_HPP_

#include "../../expect_structures.hpp"

#include <string>

namespace test::blocks
{
    static DomainParseResult domain_parse_result __attribute__((unused)) = { num_action_schemas : 4, num_predicates : 5, num_constants : 0, num_types : 1 };

    static std::string domain = R"(
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; 4 Op-blocks world
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(define (domain blocks)
  (:requirements :strips)
  (:predicates (on ?x ?y)
	           (ontable ?x)
	           (clear ?x)
	           (handempty)
	           (holding ?x))

  (:action pick-up
	     :parameters (?x)
	     :precondition (and (clear ?x)
                            (ontable ?x)
                            (handempty))
	     :effect (and (not (ontable ?x))
		              (not (clear ?x))
		              (not (handempty))
		              (holding ?x)))

  (:action put-down
	     :parameters (?x)
	     :precondition (holding ?x)
	     :effect (and (not (holding ?x))
		              (clear ?x)
		              (handempty)
		              (ontable ?x)))
  (:action stack
	     :parameters (?x ?y)
	     :precondition (and (holding ?x)
                            (clear ?y))
	     :effect (and (not (holding ?x))
		              (not (clear ?y))
		              (clear ?x)
		              (handempty)
		              (on ?x ?y)))
  (:action unstack
	     :parameters (?x ?y)
	     :precondition (and (on ?x ?y)
                            (clear ?x)
                            (handempty))
	     :effect (and (holding ?x)
		              (clear ?y)
		              (not (clear ?x))
		              (not (handempty))
		              (not (on ?x ?y)))))

)";
}  // namespace test

#endif  // MIMIR_TEST_INSTANCES_BLOCKS_DOMAIN_HPP_
