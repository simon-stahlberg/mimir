(define (domain delivery)
    (:requirements :equality :typing :negative-preconditions)
    (:types
     cell - object
     locatable - object
     package - locatable
     truck - locatable
    )

    (:predicates
     (at ?x1 - locatable ?x2 - cell)
     (at-truck ?x1 - cell)
     (carrying ?x1 - package)
     (empty)
     (adjacent ?x1 - cell ?x2 - cell)
    )

    (:action pick-package
     :parameters (?p - package ?x - cell)
     :precondition (and (at ?p ?x) (at-truck ?x) (empty))
     :effect (and (not (at ?p ?x)) (not (empty)) (carrying ?p))
    )

    (:action drop-package
     :parameters (?p - package ?x - cell)
     :precondition (and (at-truck ?x) (carrying ?p))
     :effect (and (empty) (not (carrying ?p)) (at ?p ?x))
    )

    (:action move
     :parameters (?from - cell ?to - cell)
     :precondition (and (adjacent ?from ?to) (at-truck ?from) (not (= ?from ?to)))
     :effect (and (not (at-truck ?from)) (at-truck ?to))
    )
)
