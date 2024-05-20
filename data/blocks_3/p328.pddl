;; blocks=3, percentage_new_tower=20, out_folder=., instance_id=328, seed=28

(define (problem blocksworld-328)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 b2 b3 - object)
 (:init
    (clear b3)
    (on b3 b1)
    (on b1 b2)
    (on-table b2))
 (:goal  (and
    (clear b1)
    (on b1 b3)
    (on b3 b2)
    (on-table b2))))
