;; blocks=3, percentage_new_tower=10, out_folder=., instance_id=272, seed=2

(define (problem blocksworld-272)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 b2 b3 - object)
 (:init
    (clear b3)
    (on-table b3)
    (clear b2)
    (on b2 b1)
    (on-table b1))
 (:goal  (and
    (clear b2)
    (on b2 b3)
    (on b3 b1)
    (on-table b1))))
