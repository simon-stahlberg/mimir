;; blocks=3, percentage_new_tower=40, out_folder=., instance_id=332, seed=2

(define (problem blocksworld-332)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 b2 b3 - object)
 (:init
    (clear b1)
    (on-table b1)
    (clear b2)
    (on-table b2)
    (clear b3)
    (on-table b3))
 (:goal  (and
    (clear b3)
    (on b3 b2)
    (on b2 b1)
    (on-table b1))))
