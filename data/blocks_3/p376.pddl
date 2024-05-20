;; blocks=4, percentage_new_tower=0, out_folder=., instance_id=376, seed=16

(define (problem blocksworld-376)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 b2 b3 b4 - object)
 (:init
    (clear b4)
    (on b4 b2)
    (on b2 b3)
    (on b3 b1)
    (on-table b1))
 (:goal  (and
    (clear b3)
    (on b3 b4)
    (on b4 b2)
    (on b2 b1)
    (on-table b1))))
