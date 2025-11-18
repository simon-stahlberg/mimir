(define (domain fuel-adl)
    (:requirements :numeric-fluents :typing :universal-preconditions)

    (:types vehicle)

    (:functions
        (fuel-level-max)
        (fuel-level ?v - vehicle)
    )

    (:action fuel-vehicle
        :parameters (?v - vehicle)
        :precondition (< (fuel-level ?v) (fuel-level-max))
        :effect (increase (fuel-level ?v) 1)
    )
)
