;; blocks=3, percentage_new_tower=20, out_folder=., instance_id=314, seed=14

(define (problem blocksworld-314)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 b2 b3 - object)
 (:init
    (clear b2)
    (on b2 b1)
    (on-table b1)
    (clear b3)
    (on-table b3))
 (:goal  (and
    (clear b2)
    (on b2 b1)
    (on b1 b3)
    (on-table b3))))
