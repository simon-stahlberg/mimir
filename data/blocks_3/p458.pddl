;; blocks=4, percentage_new_tower=40, out_folder=., instance_id=458, seed=8

(define (problem blocksworld-458)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 b2 b3 b4 - object)
 (:init
    (clear b3)
    (on-table b3)
    (clear b1)
    (on b1 b2)
    (on b2 b4)
    (on-table b4))
 (:goal  (and
    (clear b4)
    (on-table b4)
    (clear b2)
    (on-table b2)
    (clear b1)
    (on b1 b3)
    (on-table b3))))
