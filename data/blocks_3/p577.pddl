;; blocks=5, percentage_new_tower=40, out_folder=., instance_id=577, seed=7

(define (problem blocksworld-577)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 b2 b3 b4 b5 - object)
 (:init
    (clear b3)
    (on b3 b1)
    (on b1 b4)
    (on-table b4)
    (clear b2)
    (on-table b2)
    (clear b5)
    (on-table b5))
 (:goal  (and
    (clear b5)
    (on b5 b4)
    (on b4 b1)
    (on b1 b2)
    (on-table b2)
    (clear b3)
    (on-table b3))))
