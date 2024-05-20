;; blocks=4, percentage_new_tower=20, out_folder=., instance_id=428, seed=8

(define (problem blocksworld-428)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 b2 b3 b4 - object)
 (:init
    (clear b4)
    (on b4 b2)
    (on b2 b1)
    (on b1 b3)
    (on-table b3))
 (:goal  (and
    (clear b1)
    (on b1 b2)
    (on-table b2)
    (clear b4)
    (on b4 b3)
    (on-table b3))))
