(define (domain fuel-adl)
    (:requirements :numeric-fluents :typing :universal-preconditions)

    (:types
        vehicle location - object
        fuel-station - location
    )

    (:predicates
        (at ?v - vehicle ?l - location)
    )

    (:functions
        (fuel-level-max)
        (fuel-level ?v - vehicle)
    )

    (:action drive-vehicle
        :parameters (?v - vehicle ?from - location ?to - location)
        :precondition (and (at ?v ?from) (> (fuel-level ?v) 0))
        :effect (and (not (at ?v ?from)) (at ?v ?to) (decrease (fuel-level ?v) 1))
    )

    (:action fuel-vehicle
        :parameters (?v - vehicle ?l - fuel-station)
        :precondition (and (at ?v ?l) (< (fuel-level ?v) (fuel-level-max)))
        :effect (increase (fuel-level ?v) 1)
    )
)
