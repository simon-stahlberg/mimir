;; blocks=2, percentage_new_tower=40, out_folder=., instance_id=214, seed=4

(define (problem blocksworld-214)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 b2 - object)
 (:init
    (clear b1)
    (on b1 b2)
    (on-table b2))
 (:goal  (and
    (clear b2)
    (on-table b2)
    (clear b1)
    (on-table b1))))
