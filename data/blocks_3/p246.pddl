;; blocks=3, percentage_new_tower=0, out_folder=., instance_id=246, seed=6

(define (problem blocksworld-246)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 b2 b3 - object)
 (:init
    (clear b3)
    (on b3 b2)
    (on b2 b1)
    (on-table b1))
 (:goal  (and
    (clear b1)
    (on b1 b3)
    (on b3 b2)
    (on-table b2))))
