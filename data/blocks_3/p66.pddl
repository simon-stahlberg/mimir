;; blocks=1, percentage_new_tower=20, out_folder=., instance_id=66, seed=6

(define (problem blocksworld-66)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 - object)
 (:init
    (clear b1)
    (on-table b1))
 (:goal  (and
    (clear b1)
    (on-table b1))))
