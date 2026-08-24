;; cars=2, locations=3, seed=10, instance_id=100, out_folder=.

(define (problem ferry-100)
 (:domain ferry)
 (:requirements :typing :strips :negative-preconditions)
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
 (:goal  (and (at car1 loc1)
   (at car2 loc1))))
