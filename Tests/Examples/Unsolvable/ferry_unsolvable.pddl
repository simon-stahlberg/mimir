(define (problem ferry-100)
 (:domain ferry)
 (:objects
    car1 car2 - car
    loc1 loc3 - location
 )
 (:init
    (empty-ferry)
    (at-ferry loc3)
    (at car1 loc3)
    (at car2 loc3)
)
 (:goal (and (at-ferry loc1) (at-ferry loc3))))
