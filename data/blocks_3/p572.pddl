;; blocks=5, percentage_new_tower=40, out_folder=., instance_id=572, seed=2

(define (problem blocksworld-572)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 b2 b3 b4 b5 - object)
 (:init
    (clear b4)
    (on b4 b5)
    (on-table b5)
    (clear b3)
    (on b3 b1)
    (on b1 b2)
    (on-table b2))
 (:goal  (and
    (clear b4)
    (on-table b4)
    (clear b5)
    (on b5 b3)
    (on b3 b2)
    (on b2 b1)
    (on-table b1))))
