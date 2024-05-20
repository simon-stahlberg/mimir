;; blocks=3, percentage_new_tower=0, out_folder=., instance_id=256, seed=16

(define (problem blocksworld-256)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 b2 b3 - object)
 (:init
    (clear b3)
    (on b3 b1)
    (on b1 b2)
    (on-table b2))
 (:goal  (and
    (clear b2)
    (on b2 b1)
    (on b1 b3)
    (on-table b3))))
