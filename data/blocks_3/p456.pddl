;; blocks=4, percentage_new_tower=40, out_folder=., instance_id=456, seed=6

(define (problem blocksworld-456)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 b2 b3 b4 - object)
 (:init
    (clear b1)
    (on-table b1)
    (clear b2)
    (on b2 b3)
    (on b3 b4)
    (on-table b4))
 (:goal  (and
    (clear b3)
    (on b3 b4)
    (on-table b4)
    (clear b2)
    (on b2 b1)
    (on-table b1))))
