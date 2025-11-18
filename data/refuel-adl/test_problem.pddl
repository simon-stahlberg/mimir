(define (problem fuel-all)
    (:domain fuel-adl)

    (:objects v1 v2 v3 - vehicle)

    (:init
        (= (fuel-level-max) 3)
        (= (fuel-level v1) 0)
        (= (fuel-level v2) 1)
        (= (fuel-level v3) 2)
    )

    (:goal
        (forall (?v - vehicle) (= (fuel-level ?v) (fuel-level-max)))
    )
)
