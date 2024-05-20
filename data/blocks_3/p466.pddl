;; blocks=4, percentage_new_tower=40, out_folder=., instance_id=466, seed=16

(define (problem blocksworld-466)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 b2 b3 b4 - object)
 (:init
    (clear b1)
    (on-table b1)
    (clear b3)
    (on b3 b2)
    (on b2 b4)
    (on-table b4))
 (:goal  (and
    (clear b3)
    (on b3 b2)
    (on b2 b1)
    (on b1 b4)
    (on-table b4))))
