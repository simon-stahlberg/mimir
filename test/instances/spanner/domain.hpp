#ifndef MIMIR_TEST_INSTANCES_SPANNER_DOMAIN_HPP_
#define MIMIR_TEST_INSTANCES_SPANNER_DOMAIN_HPP_

#include "../../expect_structures.hpp"

#include <string>

namespace test::spanner
{
    static DomainParseResult domain_parse_result __attribute__((unused)) = { num_action_schemas : 3, num_predicates : 6, num_constants : 0, num_types : 6 };

    static std::string domain = R"(
(define (domain spanner)
    (:requirements :typing :strips)

    (:types location locatable - object
            man nut spanner - locatable)

    (:predicates (at ?m - locatable ?l - location)
                 (carrying ?m - man ?s - spanner)
                 (useable ?s - spanner)
                 (link ?l1 - location ?l2 - location)
                 (tightened ?n - nut)
                 (loose ?n - nut))

    (:action walk
            :parameters (?start - location ?end - location ?m - man)
            :precondition (and (at ?m ?start)
                               (link ?start ?end))
            :effect (and (not (at ?m ?start))
                         (at ?m ?end)))

    (:action pickup_spanner
            :parameters (?l - location ?s - spanner ?m - man)
            :precondition (and (at ?m ?l)
                               (at ?s ?l))
            :effect (and (not (at ?s ?l))
                         (carrying ?m ?s)))

    (:action tighten_nut
            :parameters (?l - location ?s - spanner ?m - man ?n - nut)
            :precondition (and (at ?m ?l)
                               (at ?n ?l)
                               (carrying ?m ?s)
                               (useable ?s)
                               (loose ?n))
            :effect (and (not (loose ?n))
                         (not (useable ?s))
                         (tightened ?n))))
)";
}  // namespace test

#endif  // MIMIR_TEST_INSTANCES_SPANNER_DOMAIN_HPP_
