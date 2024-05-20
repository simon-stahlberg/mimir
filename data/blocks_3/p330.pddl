;; blocks=3, percentage_new_tower=40, out_folder=., instance_id=330, seed=0

(define (problem blocksworld-330)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 b2 b3 - object)
 (:init
    (clear b3)
    (on b3 b1)
    (on-table b1)
    (clear b2)
    (on-table b2))
 (:goal  (and
    (clear b2)
    (on-table b2)
    (clear b1)
    (on b1 b3)
    (on-table b3))))
