(define (domain fuel)
    (:requirements :numeric-fluents)

    (:functions
        (fuel) ; amount of fuel
    )

    (:action refuel
        :parameters ()
        :precondition ()
        :effect (increase (fuel) 1)
    )
)
