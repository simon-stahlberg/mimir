;; blocks=3, percentage_new_tower=10, out_folder=., instance_id=281, seed=11

(define (problem blocksworld-281)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 b2 b3 - object)
 (:init
    (clear b3)
    (on b3 b1)
    (on b1 b2)
    (on-table b2))
 (:goal  (and
    (clear b2)
    (on b2 b3)
    (on b3 b1)
    (on-table b1))))