;; blocks=4, percentage_new_tower=10, out_folder=., instance_id=406, seed=16

(define (problem blocksworld-406)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 b2 b3 b4 - object)
 (:init
    (clear b1)
    (on b1 b3)
    (on b3 b4)
    (on b4 b2)
    (on-table b2))
 (:goal  (and
    (clear b3)
    (on b3 b2)
    (on b2 b1)
    (on b1 b4)
    (on-table b4))))
