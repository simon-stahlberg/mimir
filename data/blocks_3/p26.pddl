;; blocks=1, percentage_new_tower=0, out_folder=., instance_id=26, seed=26

(define (problem blocksworld-26)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 - object)
 (:init
    (clear b1)
    (on-table b1))
 (:goal  (and
    (clear b1)
    (on-table b1))))
