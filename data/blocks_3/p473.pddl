;; blocks=4, percentage_new_tower=40, out_folder=., instance_id=473, seed=23

(define (problem blocksworld-473)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 b2 b3 b4 - object)
 (:init
    (clear b1)
    (on-table b1)
    (clear b2)
    (on-table b2)
    (clear b3)
    (on b3 b4)
    (on-table b4))
 (:goal  (and
    (clear b4)
    (on-table b4)
    (clear b2)
    (on-table b2)
    (clear b1)
    (on-table b1)
    (clear b3)
    (on-table b3))))
