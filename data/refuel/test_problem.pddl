(define (problem fuel-5)
    (:domain fuel)

    (:init
        (= (fuel) 0)
    )

    (:goal
        (and
            (>= (fuel) 5)
        )
    )
)
