;; blocks=5, percentage_new_tower=40, out_folder=., instance_id=578, seed=8

(define (problem blocksworld-578)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 b2 b3 b4 b5 - object)
 (:init
    (clear b5)
    (on-table b5)
    (clear b4)
    (on b4 b1)
    (on b1 b3)
    (on-table b3)
    (clear b2)
    (on-table b2))
 (:goal  (and
    (clear b5)
    (on b5 b1)
    (on b1 b2)
    (on b2 b3)
    (on b3 b4)
    (on-table b4))))
