;; blocks=3, percentage_new_tower=20, out_folder=., instance_id=309, seed=9

(define (problem blocksworld-309)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 b2 b3 - object)
 (:init
    (clear b2)
    (on b2 b3)
    (on b3 b1)
    (on-table b1))
 (:goal  (and
    (clear b3)
    (on b3 b2)
    (on b2 b1)
    (on-table b1))))
