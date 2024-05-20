;; blocks=1, percentage_new_tower=10, out_folder=., instance_id=31, seed=1

(define (problem blocksworld-31)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 - object)
 (:init
    (clear b1)
    (on-table b1))
 (:goal  (and
    (clear b1)
    (on-table b1))))
