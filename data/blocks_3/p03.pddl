;; blocks=1, percentage_new_tower=0, out_folder=., instance_id=3, seed=3

(define (problem blocksworld-03)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 - object)
 (:init
    (clear b1)
    (on-table b1))
 (:goal  (and
    (clear b1)
    (on-table b1))))