;; blocks=2, percentage_new_tower=40, out_folder=., instance_id=210, seed=0

(define (problem blocksworld-210)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 b2 - object)
 (:init
    (clear b2)
    (on-table b2)
    (clear b1)
    (on-table b1))
 (:goal  (and
    (clear b1)
    (on-table b1)
    (clear b2)
    (on-table b2))))
