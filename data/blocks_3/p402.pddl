;; blocks=4, percentage_new_tower=10, out_folder=., instance_id=402, seed=12

(define (problem blocksworld-402)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 b2 b3 b4 - object)
 (:init
    (clear b2)
    (on b2 b4)
    (on b4 b1)
    (on-table b1)
    (clear b3)
    (on-table b3))
 (:goal  (and
    (clear b3)
    (on-table b3)
    (clear b2)
    (on b2 b1)
    (on b1 b4)
    (on-table b4))))
