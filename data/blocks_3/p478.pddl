;; blocks=4, percentage_new_tower=40, out_folder=., instance_id=478, seed=28

(define (problem blocksworld-478)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 b2 b3 b4 - object)
 (:init
    (clear b3)
    (on b3 b2)
    (on b2 b4)
    (on b4 b1)
    (on-table b1))
 (:goal  (and
    (clear b1)
    (on b1 b3)
    (on b3 b2)
    (on b2 b4)
    (on-table b4))))
