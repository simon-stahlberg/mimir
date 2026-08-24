;; blocks=3, percentage_new_tower=20, out_folder=., instance_id=300, seed=0

(define (problem blocksworld-300)
 (:domain blocksworld)
 (:requirements :strips :typing)
 (:objects b1 b2 b3 - object)
 (:init
    (arm-empty)
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
