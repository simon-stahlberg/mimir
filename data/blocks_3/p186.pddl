;; blocks=2, percentage_new_tower=20, out_folder=., instance_id=186, seed=6

(define (problem blocksworld-186)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 b2 - object)
 (:init
    (clear b2)
    (on b2 b1)
    (on-table b1))
 (:goal  (and
    (clear b1)
    (on-table b1)
    (clear b2)
    (on-table b2))))
