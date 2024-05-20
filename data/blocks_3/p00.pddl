;; blocks=1, percentage_new_tower=0, out_folder=., instance_id=0, seed=0

(define (problem blocksworld-00)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 - object)
 (:init
    (clear b1)
    (on-table b1))
 (:goal  (and
    (clear b1)
    (on-table b1))))
