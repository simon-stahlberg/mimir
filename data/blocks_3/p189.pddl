;; blocks=2, percentage_new_tower=20, out_folder=., instance_id=189, seed=9

(define (problem blocksworld-189)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 b2 - object)
 (:init
    (clear b2)
    (on b2 b1)
    (on-table b1))
 (:goal  (and
    (clear b2)
    (on-table b2)
    (clear b1)
    (on-table b1))))
