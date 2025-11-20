(define (problem fuel-all)
    (:domain fuel-adl)

    (:objects
        v1 v2 v3 - vehicle
        l1 - location
        fs1 - fuel-station
    )

    (:init
        (= (fuel-level-max) 3)
        (= (fuel-level v1) 0)
        (= (fuel-level v2) 1)
        (= (fuel-level v3) 2)
        (at v1 fs1)
        (at v2 l1)
        (at v3 l1)
    )

    (:goal
        (forall (?v - vehicle) (= (fuel-level ?v) (fuel-level-max)))
    )
)
