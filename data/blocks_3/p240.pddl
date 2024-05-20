;; blocks=3, percentage_new_tower=0, out_folder=., instance_id=240, seed=0

(define (problem blocksworld-240)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 b2 b3 - object)
 (:init
    (clear b1)
    (on b1 b2)
    (on b2 b3)
    (on-table b3))
 (:goal  (and
    (clear b1)
    (on b1 b3)
    (on b3 b2)
    (on-table b2))))
