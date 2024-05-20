;; blocks=1, percentage_new_tower=40, out_folder=., instance_id=105, seed=15

(define (problem blocksworld-105)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 - object)
 (:init
    (clear b1)
    (on-table b1))
 (:goal  (and
    (clear b1)
    (on-table b1))))
