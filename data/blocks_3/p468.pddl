;; blocks=4, percentage_new_tower=40, out_folder=., instance_id=468, seed=18

(define (problem blocksworld-468)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 b2 b3 b4 - object)
 (:init
    (clear b4)
    (on-table b4)
    (clear b2)
    (on b2 b3)
    (on b3 b1)
    (on-table b1))
 (:goal  (and
    (clear b4)
    (on-table b4)
    (clear b2)
    (on-table b2)
    (clear b3)
    (on b3 b1)
    (on-table b1))))
