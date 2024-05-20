;; blocks=3, percentage_new_tower=20, out_folder=., instance_id=313, seed=13

(define (problem blocksworld-313)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 b2 b3 - object)
 (:init
    (clear b2)
    (on-table b2)
    (clear b3)
    (on b3 b1)
    (on-table b1))
 (:goal  (and
    (clear b1)
    (on b1 b3)
    (on-table b3)
    (clear b2)
    (on-table b2))))
