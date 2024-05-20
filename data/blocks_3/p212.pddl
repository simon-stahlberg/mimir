;; blocks=2, percentage_new_tower=40, out_folder=., instance_id=212, seed=2

(define (problem blocksworld-212)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 b2 - object)
 (:init
    (clear b1)
    (on-table b1)
    (clear b2)
    (on-table b2))
 (:goal  (and
    (clear b2)
    (on-table b2)
    (clear b1)
    (on-table b1))))
