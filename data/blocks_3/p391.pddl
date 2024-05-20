;; blocks=4, percentage_new_tower=10, out_folder=., instance_id=391, seed=1

(define (problem blocksworld-391)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 b2 b3 b4 - object)
 (:init
    (clear b1)
    (on-table b1)
    (clear b3)
    (on b3 b4)
    (on b4 b2)
    (on-table b2))
 (:goal  (and
    (clear b2)
    (on-table b2)
    (clear b4)
    (on b4 b3)
    (on b3 b1)
    (on-table b1))))
