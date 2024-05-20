;; blocks=1, percentage_new_tower=10, out_folder=., instance_id=43, seed=13

(define (problem blocksworld-43)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 - object)
 (:init
    (clear b1)
    (on-table b1))
 (:goal  (and
    (clear b1)
    (on-table b1))))
