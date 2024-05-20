;; blocks=4, percentage_new_tower=0, out_folder=., instance_id=373, seed=13

(define (problem blocksworld-373)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 b2 b3 b4 - object)
 (:init
    (clear b2)
    (on b2 b1)
    (on b1 b3)
    (on b3 b4)
    (on-table b4))
 (:goal  (and
    (clear b1)
    (on b1 b2)
    (on b2 b3)
    (on b3 b4)
    (on-table b4))))
