;; blocks=4, percentage_new_tower=10, out_folder=., instance_id=398, seed=8

(define (problem blocksworld-398)
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
    (clear b3)
    (on b3 b2)
    (on b2 b4)
    (on b4 b1)
    (on-table b1))))
