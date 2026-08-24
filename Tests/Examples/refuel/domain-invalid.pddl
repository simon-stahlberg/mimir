(define (domain fuel)
    (:requirements :numeric-fluents)

    (:functions
        (fuel) ; amount of fuel
    )

    (:action refuel
        :parameters ()
        :precondition ()
        :effect (increase (fake-fuel) 1)
    )
)
