;; blocks=2, percentage_new_tower=40, out_folder=., instance_id=213, seed=3

(define (problem blocksworld-213)
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
