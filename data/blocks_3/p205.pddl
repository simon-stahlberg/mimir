;; blocks=2, percentage_new_tower=20, out_folder=., instance_id=205, seed=25

(define (problem blocksworld-205)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 b2 - object)
 (:init
    (clear b2)
    (on b2 b1)
    (on-table b1))
 (:goal  (and
    (clear b2)
    (on b2 b1)
    (on-table b1))))
