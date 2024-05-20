;; blocks=3, percentage_new_tower=20, out_folder=., instance_id=307, seed=7

(define (problem blocksworld-307)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 b2 b3 - object)
 (:init
    (clear b1)
    (on-table b1)
    (clear b2)
    (on b2 b3)
    (on-table b3))
 (:goal  (and
    (clear b2)
    (on b2 b3)
    (on b3 b1)
    (on-table b1))))
