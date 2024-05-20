;; blocks=4, percentage_new_tower=40, out_folder=., instance_id=450, seed=0

(define (problem blocksworld-450)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 b2 b3 b4 - object)
 (:init
    (clear b3)
    (on b3 b4)
    (on-table b4)
    (clear b2)
    (on-table b2)
    (clear b1)
    (on-table b1))
 (:goal  (and
    (clear b3)
    (on b3 b2)
    (on-table b2)
    (clear b4)
    (on b4 b1)
    (on-table b1))))
