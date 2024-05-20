;; blocks=4, percentage_new_tower=10, out_folder=., instance_id=410, seed=20

(define (problem blocksworld-410)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 b2 b3 b4 - object)
 (:init
    (clear b3)
    (on b3 b4)
    (on b4 b1)
    (on b1 b2)
    (on-table b2))
 (:goal  (and
    (clear b2)
    (on b2 b3)
    (on-table b3)
    (clear b1)
    (on b1 b4)
    (on-table b4))))
