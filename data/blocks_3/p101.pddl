;; blocks=1, percentage_new_tower=40, out_folder=., instance_id=101, seed=11

(define (problem blocksworld-101)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 - object)
 (:init
    (clear b1)
    (on-table b1))
 (:goal  (and
    (clear b1)
    (on-table b1))))