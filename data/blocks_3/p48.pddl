;; blocks=1, percentage_new_tower=10, out_folder=., instance_id=48, seed=18

(define (problem blocksworld-48)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 - object)
 (:init
    (clear b1)
    (on-table b1))
 (:goal  (and
    (clear b1)
    (on-table b1))))
