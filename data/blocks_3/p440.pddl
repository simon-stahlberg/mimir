;; blocks=4, percentage_new_tower=20, out_folder=., instance_id=440, seed=20

(define (problem blocksworld-440)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 b2 b3 b4 - object)
 (:init
    (clear b4)
    (on-table b4)
    (clear b2)
    (on-table b2)
    (clear b3)
    (on b3 b1)
    (on-table b1))
 (:goal  (and
    (clear b3)
    (on b3 b2)
    (on b2 b1)
    (on b1 b4)
    (on-table b4))))
