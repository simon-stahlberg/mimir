;; blocks=3, percentage_new_tower=20, out_folder=., instance_id=303, seed=3

(define (problem blocksworld-303)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 b2 b3 - object)
 (:init
    (clear b2)
    (on-table b2)
    (clear b1)
    (on b1 b3)
    (on-table b3))
 (:goal  (and
    (clear b2)
    (on b2 b3)
    (on-table b3)
    (clear b1)
    (on-table b1))))
